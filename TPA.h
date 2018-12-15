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
}

double sum(std::vector<double>);
stringArray split(std::string, char);
bool in(char, std::string);
std::string extension(std::string);

class Matrix{
private:
    std::vector<std::vector<double>> element;
public:
    std::vector<unsigned> size;
    Matrix(unsigned, unsigned);
    std::vector<double>& operator[])(size_t);
    std::vector<double> column(unsigned);
    std::vector<double> row(unsigned);
};

class PreferenceData{
public:
    PreferenceData(std::string);
    int n_trials;
    std::set<int> sound_id;
    std::set<int> soudn_freq;
    bool isOriginal;
private:
    std::ifstream ifs;
    std::vector<std::vector<string>> trials;
    std::vector<string> data;
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
    stringArray filenameList;
};

class TPA{
public:
    std::vector<std::vector<double>> PI;
    std::vector<std::vector<double>> inflow;
    std::vector<std::vector<double>> outflow;
    TPA(const char*);//dir name
    TPA(std::string);//dir name
    void reckon(std::string);//ratName
private:
    bool calculated;
    bool generated;
    std::string name;
    void generateIntermediateFile(std::string);//dir name
    void calcPI();
    void writePIcsv();
};