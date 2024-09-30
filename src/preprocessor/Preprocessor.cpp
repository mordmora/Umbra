#include"Preprocessor.h"
#include <experimental/optional>
#include <fstream>
#include<iostream>


namespace umbra {
    
#define INCLUDE_KEYWORD "use"

using namespace std::experimental;

SourceLocation::SourceLocation(const std::string &path, unsigned int col, unsigned int line)
    : path(std::move(path)), col(col), line(line) {}

File::File(const std::string &fileName, SourceLocation location,
           bool resolved, const optional<std::string> &callBy)
    :  fileName(std::move(fileName)), location(std::move(location)),
      resolved(false), callBy(std::move(callBy)) {}

bool Preprocessor::contains(File f){
    auto it = included.find(f);
    return !(it == included.end());
}

Preprocessor::Preprocessor(File file) : origin(file) {}

std::unique_ptr<std::string> Preprocessor::readfile(std::string &filename) {
    std::ifstream f(filename, std::ios::in | std::ios::binary); // open input file in only read mode
    if (!f.is_open()) {
        throw std::runtime_error("File error.");
    }
    std::filebuf *buf = f.rdbuf();
    std::size_t size = buf->pubseekoff(0, f.end, f.in);
    buf->pubseekpos(0, f.in);
    char *out = new char[size + 1];
    buf->sgetn(out, size);
    out[size] = '\0';
    std::string result(out);
    delete[] out;
    return std::make_unique<std::string>(std::move(result));
}

std::pair<std::string, std::size_t> getWord(const std::string &input_str, std::size_t index) {
    const char BLANK = ' ';
    const char TAB = '\t';
    const char NEWLINE = '\n';
    std::string out;
    // Saltar espacios en blanco y tabulaciones iniciales
    while (index < input_str.size() && (input_str[index] == BLANK || input_str[index] == TAB)) {
        index++;
    }
    // Extraer la palabra
    while (index < input_str.size() && input_str[index] != BLANK && input_str[index] != TAB && input_str[index] != NEWLINE) {
        out += input_str[index];
        index++;
    }
    return {out, index};
}

bool Preprocessor::fileExist(const std::ifstream &file){
    return file.good();
}



std::string Preprocessor::includeFiles(const std::string& filename, int level){

    std::ifstream file(filename.c_str());
    if(!fileExist(file)){
        throw std::runtime_error("El archivo especificado no se pudo localizar");
    }
    File f = File(filename, origin.location, false, origin.callBy);

    if(contains(f)){
        return "";
    }

    included.insert(f);
    std::string result;
    std::string line;
    while(std::getline(file, line)){
        std::size_t loc = 0;
        std::string word;
        std::tie(word, loc) = getWord(line, loc);

        if(word != INCLUDE_KEYWORD){
            std::cout << line << std::endl;
        } else {
            std::string included_file;
            std::tie(included_file, loc) = getWord(line, loc);

            std::cout << "Incluyendo " << included_file << "desde " << std::endl;

            result += includeFiles(included_file, level + 1);
        }
    }
    file.close();
    return result;
}

};