#include "TPA.h"

using namespace std;

Matrix::Matrix(){
    ;
}

Matrix::Matrix(unsigned row, unsigned column){
    for(auto i = 0; i<row; ++i){
        vector<double> temp(column, 0.0);
        element.push_back(temp);
    }
    size.push_back(row);
    size.push_back(column);
}

Matrix& Matrix::operator=(const Matrix &rh){
    size = rh.size;
    element = rh.element;
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
    cout << "Opening:" << dataname << endl << flush;
    ifs.open(dataname);
    if(ifs.is_open()==true){
        dataload();
    }else{
        cout << "error: can't open " << dataname << endl;
    }
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

TPA::TPA(string analyzedir):generated(false),analyze_directory(analyzedir){
    os::mkdir(analyzedir);
    os::mkdir(analyzedir+"/PI");
    os::mkdir(analyzedir+"/Inflow");
    os::mkdir(analyzedir+"/Outflow");
}

void TPA::generateIntermediateFile(string datadir, string analyzedir){
    if(os::exist(analyzedir+"/outputTransferdata.data")){
        cout << "Analyzed file (outputTransferdata.data) exists!" << endl;
        generated = true;
        return;
    }
    ofstream ofs(analyzedir + "/outputTransferdata.data");
    Directory curdir(datadir);
    for(auto f: curdir.filenameList){
        if(extension(f)!="txt"){
            cout << "ignore file: " << extension(f) << endl;
        }else{
            PreferenceData pf(datadir + "/"+ f);
            cout << datadir + "/" + f << endl;
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
void TPA::calcPI(string analyzedir){
    /* calculate PIs of txt files and save to member variables: PI/inflow/outflow */
    if(!generated){
        return;
    }
    ifstream ifs(analyzedir + "/outputTransferdata.data");

    Matrix* pm;
    string ratname,date,temp;
    map<string, vector<Datematrix>> tmpDataStorage; 
    //tmpDataStorage[ratname] = Datematrix{date(string), matrix}

    bool isNewTrial = false;
    int trial = 1;
    while(getline(ifs,temp)){
        if(temp[0]=='#'){
            if(temp[1]!=','){//header
                if(isNewTrial){//new file loop begins (for the very first time this is skipped)
                    Datematrix dm;
                    dm.date = date;
                    dm.matrix = *pm;
                    tmpDataStorage[ratname].push_back(dm);
                    delete pm;

                    isNewTrial = false;
                }
                //Extract rat name and date
                string fname = split(temp, '.')[0];
                auto temp_name = split(fname, '_');
                ratname = temp_name[0].substr(1);
                date = "";
                for(int i = 1; i<temp_name.size(); ++i){
                    date += temp_name[i];
                }
                //ratname = M-01, date=1901041920
                trial = 1;//initialization
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
    {
        Datematrix dm;
        dm.date = date;
        dm.matrix = *pm;
        tmpDataStorage[ratname].push_back(dm);
        delete pm;
    }
    //PIのけいさん
    for(auto datemap: tmpDataStorage){
        for(auto datedata : datemap.second){
            auto k = datedata.matrix;
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
            Datedata dd_pi, dd_in, dd_out;
            dd_pi.date = datedata.date;
            dd_in.date = datedata.date;
            dd_out.date = datedata.date;

            dd_pi.data = _PI;
            dd_in.data = _inflow;
            dd_out.data = _outflow;

            PI[datemap.first].push_back(dd_pi);
            inflow[datemap.first].push_back(dd_in);
            outflow[datemap.first].push_back(dd_out);
        }
    }
    calculated = true;
}

void TPA::writeCSVdata(){
    if(!calculated){
        return;
    }
    for(auto pi:PI){
        ofstream ofs(analyze_directory+ "/PI/" + pi.first+".csv");
        for(auto dd : pi.second){
            ofs << dd.date << ",";
            for(auto i=dd.data.begin(); i<dd.data.end(); ++i){
                ofs << *i;
                if(i==(dd.data.end()-1)){
                    ofs << endl;
                }else{
                    ofs << ",";
                }
            }
        }
    }
    for(auto _in:inflow){
        ofstream ofs(analyze_directory+ "/Inflow/" + _in.first +".csv");
        for(auto dd : _in.second){
            ofs << dd.date << ",";
            for(auto i=dd.data.begin(); i<dd.data.end(); ++i){
                ofs << *i;
                if(i==(dd.data.end()-1)){
                    ofs << endl;
                }else{
                    ofs << ",";
                }
            }
        }
    }
    for(auto _out:outflow){
        ofstream ofs(analyze_directory+ "/Outflow/" + _out.first+ ".csv");
        for(auto dd : _out.second){
            ofs << dd.date << ",";
            for(auto i=dd.data.begin(); i<dd.data.end(); ++i){
                ofs << *i;
                if(i==(dd.data.end()-1)){
                    ofs << endl;
                }else{
                    ofs << ",";
                }
            }
        }
    }  
}

void TPA::reckon(string datadir){
    generateIntermediateFile(datadir, this->analyze_directory);
    calcPI(this->analyze_directory);
    writeCSVdata();
}

void os::mkdir(std::string foldername){
    system((string("mkdir ")+foldername).c_str());
}

bool os::exist(std::string str){
    ifstream ifs(str);
    return ifs.is_open();
}
