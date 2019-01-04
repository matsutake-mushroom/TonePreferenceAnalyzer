#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>

typedef std::vector<std::string> stringArray;
typedef struct _Transfer{
    int start;
    int goal;
} Transfer;

typedef struct _datedata{
    std::string date;
    std::vector<double> data;
} Datedata;


double sum(std::vector<double>);
stringArray split(std::string, char);
bool in(char, std::string);
std::string extension(std::string);

class Matrix{
private:
    std::vector<std::vector<double>> element;
public:
    std::vector<unsigned> size;
    Matrix();
    Matrix(unsigned, unsigned);
    Matrix& operator=(const Matrix &rh);
    std::vector<double>& operator[](size_t);
    std::vector<double> column(unsigned);
    std::vector<double> row(unsigned);
};

typedef struct _datematrix{
    std::string date;
    Matrix matrix;
} Datematrix;

class PreferenceData{
public:
    PreferenceData(std::string);
    int n_trials;
    std::set<int> sound_id;
    std::set<int> sound_freq;
    bool isOriginal;
private:
    std::ifstream ifs;
    std::vector<std::vector<std::string>> trials;
    std::vector<std::string> data;
public:
    std::vector<Transfer> transferData;
    void dataload();
    void extractTransferData(const char);
};

class Directory{
public:
    Directory();
    Directory(std::string);
private:
    std::string data_dir;
    void scanFilenames(std::string);
public:
    stringArray filenameList;
};

class TPA{
public:
    std::map<std::string, std::vector<Datedata>> PI;
    std::map<std::string, std::vector<Datedata>> inflow;
    std::map<std::string, std::vector<Datedata>> outflow;
    TPA(std::string);//analyzedir
    std::string analyze_directory;
    void reckon(std::string);//datadir
private:
    bool calculated;
    bool generated;
    void generateIntermediateFile(std::string, std::string);//datadir, analyzedir
    void calcPI(std::string);//analyzedir
    void writeCSVdata();
};

namespace os{
    void mkdir(std::string);
    bool exist(std::string);

}
