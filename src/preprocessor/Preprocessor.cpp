#include"Preprocessor.h"
#include <optional>
#include <fstream>
#include<iostream>
#include<memory>
#include<vector>

/*
 * TODO:
 * - Ajustar una ruta relativa virtual para no indicar de forma explicita la ubicacion de los archivos
 *   en las cabeceras, sino que se tome tomo ruta principal la carpeta donde esta ubicado el codigo fuente
 *
 * - Crear herramientas de diagnostico para garantizar y optimizar el manejo de archivos incluidos
 *
 * */


namespace umbra {

#define INCLUDE_KEYWORD "use"


SourceLocation::SourceLocation(const std::string &path, unsigned int col, unsigned int line)
    : path(std::move(path)), col(col), line(line) {}

SourceLocation::SourceLocation(){}

File::File(const std::string &fileName, SourceLocation location,
           bool resolved, const std::optional<std::string> &callBy)
    :  fileName(fileName), location(location),
      resolved(resolved), callBy(callBy) {}

File::File(){}

bool Preprocessor::contains(File f){
    return included.find(f.fileName) != included.end();
}

Preprocessor::Preprocessor(File file) : origin(file) {
    includeFiles(file, 0);
}

std::pair<std::string, std::size_t> Preprocessor::getWord(const std::string &input_str, std::size_t index) {
    const char BLANK = ' ';
    const char TAB = '\t';
    const char NEWLINE = '\n';
    std::string out;
    while (index < input_str.size() && (input_str[index] == BLANK || input_str[index] == TAB)) {
        index++;
    }
    while (index < input_str.size() && input_str[index] != BLANK && input_str[index] != TAB && input_str[index] != NEWLINE) {
        out += input_str[index];
        index++;
    }
    return {out, index};
}

bool Preprocessor::fileExist(const std::ifstream &file){
    return file.good();
}

void Preprocessor::markAsResolved(File inputFile) {
    auto it = included.find(inputFile.fileName); // Buscar usando el nombre del archivo
    if (it != included.end()) {
        File newFile = it->second; // Copiar el archivo
        newFile.resolved = true;   // Cambiar el estado
        included[inputFile.fileName] = newFile; // Actualizar el map
    }
}

std::string Preprocessor::includeFiles(File inputFile, int level){

    File f;
    std::ifstream file(inputFile.fileName);
    std::cout<<inputFile.fileName << std::endl;
    if(!fileExist(file)){
        throw std::runtime_error("El archivo especificado no se pudo localizar");
    }

    if(contains(inputFile)){
        return "";
    }

    included.insert({inputFile.fileName, inputFile});
    std::string result;
    std::string line;
    std::size_t row = 0;
    while(std::getline(file, line)){
        row++;
        std::size_t col = 0;
        std::string word;
        std::tie(word, col) = getWord(line, col);

        if(word != INCLUDE_KEYWORD){
            std::cout << line << std::endl;
        } else {
            std::string included_file;
            std::tie(included_file, col) = getWord(line, col);
            SourceLocation location = {inputFile.fileName, col - included_file.length(), row};
            f = {included_file, location, false, inputFile.fileName};
            result += includeFiles(f, level + 1);
        }
    }

    markAsResolved(inputFile);
    file.close();
    return result;
}

void Preprocessor::diagnosticTool(){
    for(auto it : included){
        std::cout << it.first << " Resolved: " << it.second.resolved << std::endl;
    }
}

};

