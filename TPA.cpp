#include "TPA.h"

using namespace std;


Matrix::Matrix(unsigned row, unsigned column){
    for(auto i = 0; i<row; ++i){
        vector<double> temp(column, 0.0);
        element.push_back(temp);
    }
    size.push_back(row);
    size.push_back(column);
}

vector<double>& Matrix::operator[](size_t t){
    return element[t];
}

vector<double> Matrix::column(unsigned c){
    vector<double> temp;
    for(auto i = 0; i<size[0]; ++i){
        temp.push_back(element[i][c]);
    }
    return temp;
}
vector<double> Matrix::row(unsigned r){
    return element[r];
}

double sum(vector<double> array){
    double ret = 0.0;
    for(auto i : array){
        ret += i;
    }
    return ret;
}

stringArray split(string input, char delim)
{
    stringArray elems;
    string item;
    for (char ch: input) {
        if (ch == delim) {
            if (!item.empty())
                elems.push_back(item);
            item.clear();
        }
        else {
            item += ch;
        }
    }
    if (!item.empty()){
        elems.push_back(item);
    }
    return elems;
}

bool in(char key, string line){
    for(auto c : line){
        if(c == key){
            return true;
        }
    }
    return false;
}

string extension(string filename){
    stringArray x = split(filename, '.');
    if(x.size()==1){
        return "";
    }else{
        return x[x.size()-1];
    }
}

PreferenceData::PreferenceData(string dataname){
    ifs.open(dataname);
    dataload();
}

void PreferenceData::dataload(){
    string buffer, trial;
    bool isNewTrial = false;
    isOriginal = false;
    
    while(!ifs.eof()){
        getline(ifs,buffer);
        if(buffer[0] == '#'){
            if(!isNewTrial){
                if(data.size()!=0){
                    trials.push_back(data);
                    data.clear();
                }
                isNewTrial = true;
            }
            //Original 限定
            if(in('(', buffer)){
                stringArray sa = split(buffer, '(');
                isOriginal = true;
                for(string s : sa){
                    if(s[0]!='#'){//最初以外
                        sound_freq.insert(stoi(split(s, ' ')[0]));//Hz
                    }
                }
                
            }
            //音IDを列挙する列を取得
            if(!in('(', buffer) && !in(':', buffer)){
                auto elem = split(buffer,',');
                for(auto e: elem){
                    if(e[0]!='#'){
                        sound_id.insert(stoi(e));
                    }
                }
            }
        }else{
            if(isNewTrial){
                isNewTrial = false;
            }
        }
        data.push_back(buffer);//改行を付けて追加
    }

    //最後の行は削除
    data.erase(data.end()-1);//""が入っているぽい
    data.erase(data.end()-1);//最後の行

    trials.push_back(data);
    //総数を登録
    n_trials = trials.size();
}

//分割済みデータから移動データをつくる関数
void PreferenceData::extractTransferData(const char delim){
    transferData.clear();
    for(auto i = 0; i<n_trials; ++i){
        vector<string> each_trial_data = trials[i];
        Transfer t;
        t.start = stoi(split(each_trial_data[3], delim)[1]);
        t.goal  = stoi(split(*(each_trial_data.end()-1), delim)[1]);
        transferData.push_back(t);
    }
}



void Directory::scanFilenames(string _data_dir){
    string temporary_file = ".filenameList.temp";
    string command = "ls " + _data_dir + " >" + temporary_file;
    system(command.c_str());
    ifstream ifs(temporary_file);
    if(!ifs.fail()){
        string buffer;
        while(getline(ifs,buffer)){
            if(buffer!=temporary_file){
                filenameList.push_back(buffer);
            }
        }
    }
    //後片付け
    command = "rm " + temporary_file;
    system(command.c_str());
}

Directory::Directory(){
    data_dir = string(".");
    scanFilenames(data_dir);
}

Directory::Directory(string _data_dir){
    data_dir = _data_dir;
    scanFilenames(data_dir);
}

TPA::TPA():generated(false){
    system::mkdir("analyze");
    system::mkdir("analyze/PI");
    system::mkdir("analyze/Inflow");
    system::mkdir("analyze/Outflow");
}

void TPA::generateIntermediateFile(string directory_name){
    ofstream ofs("analyze/outputTransferdata.data");
    Directory curdir(directory_name);
    for(auto f: curdir.filenameList){
        if(extension(f)!="txt"){
            cout << "ignore file: " << extension(f) << endl;
        }else{
            PreferenceData pf(f);
            switch(f[0]){
                case 'C':
                case 'M':
                    cout << "Chord: " << f << endl;
                    pf.extractTransferData(' ');
                    break;
                case 'P':
                case 'R':
                    cout << "Pitch/Rhythm: " << f << endl;
                    pf.extractTransferData(',');
                    break;
                default:
                    cout << "Unknown txt file: "<< f << endl;
                    break;
            }
            
            ofs << "#" << f << endl;
            ofs << "#" << flush;
            for(auto s: pf.sound_id){
                ofs << "," << s << flush;
            }
            ofs << endl;

            if(pf.isOriginal){//identifkHz -> sound_id
                //cout << "ORIG" << endl;
                for(auto t: pf.transferData){
                    int count = 0;
                    int start = 0;
                    int goal = 0;
                    for(auto s : pf.sound_freq){
                        if(s/1000==t.start){
                            start = count;
                        }
                        if(s/1000==t.goal){
                            goal = count;
                        }
                        count++;
                    }
                    ofs << start << " " << goal << endl;
                }
            }else{
                for(auto t : pf.transferData){
                    ofs << t.start << " " << t.goal << endl;
                }
            }
        }
    }
    ofs.close();
    generated = true;
}
void TPA::calcPI(string ratname){
    if(!generated){
        return;
    }
    ifstream ifs("analyze/outputTransferdata.data");
    string temp;
    vector<Matrix> kaiseki;
    Matrix* pm;
    bool isNewTrial = false;
    int trial = 1;
    while(getline(ifs,temp)){
        if(temp[0]=='#'){
            if(temp[1]!=','){//header
                if(isNewTrial){
                    kaiseki.push_back(*pm);
                    delete pm;
                    isNewTrial = false;
                }
                trial = 1;
            }
            if(temp[1]==','){//2nd row
                int size = stoi(*((split(temp,',')).end()-1)) + 1;
                pm = new Matrix(size,size);
                isNewTrial = true;
            }
        }else{
            if(trial<=5){
                trial++;//habituation
            }else{
                stringArray sa = split(temp, ' ');
                (*pm)[stoi(sa[0])][stoi(sa[1])] += 1.0;
            }
        }
    }
    //後始末
    kaiseki.push_back(*pm);
    delete pm;

    //PIのけいさん
    for(auto k: kaiseki){
        vector<double> _PI(k.size[0],0.0);
        vector<double> _inflow(k.size[0],0.0);
        vector<double> _outflow(k.size[0],0.0);
        double sum_all = 0.0;
        for(auto i=0; i<k.size[0]; ++i){
            sum_all += sum(k.row(i));
        }
        
        for(auto i=0; i<k.size[0]; ++i){
            double s_and_g_is_x = k[i][i];
            double g_is_x = sum(k.column(i));
            double s_isnot_x = sum_all - sum(k.row(i));
            double s_is_x = sum(k.row(i));
            
            _inflow[i] = (g_is_x - s_and_g_is_x)/s_isnot_x;
            _outflow[i] = (s_is_x - s_and_g_is_x)/s_is_x;
            _PI[i] = _inflow[i] - _outflow[i];
        }
        PI.push_back(_PI);
        inflow.push_back(_inflow);
        outflow.push_back(_outflow);
    }

    calculated = true;
}

void TPA::writeCSVdata(string ratname){
    if(!calculated){
        return;
    }

    ofstream ofs(string("PI/")+ ratname);
    for(auto p:PI){
        for(auto i=p.begin(); i<p.end(); ++i){
            ofs << *i;
            if(i==(p.end()-1)){
                ofs << endl;
            }else{
                ofs << ",";
            }
        }
    }
}

void TPA::reckon(string ratname){
    generateIntermediateFile(name);
    calcPI();
    writePIcsv(ratname + string("_pi.csv"));
}

void system::mkdir(std::string foldername){
    system((string("mkdir ")+foldername).c_str());
}