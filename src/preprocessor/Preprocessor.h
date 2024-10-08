#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <unordered_map>
#include <string>
#include<experimental/optional>
#include<memory>

namespace umbra {
/*
Declaration file for Umbra preprocessor
*/
using namespace std::experimental;

/*
Estructura para mantener un control de donde se incluye el archivo
*/

struct SourceLocation {
    std::string path;
    unsigned int col;
    unsigned int line;
    SourceLocation(const std::string& path, unsigned int col, unsigned int line);
    SourceLocation();
};

/*
Para un mejor control, se define la estructure @File para tener una
representación mas completa del archivo, desde su ubicación en el
codigo fuente, su nombre y si fue resuelto o no.

Opcionalmente guarda el nombre del archivo que lo llamó en algún
lugar del codigo
*/

struct File {
    std::string fileName;
    SourceLocation location;
    bool resolved;
    optional<std::string> callBy = nullopt;


    File();
    
    File(const std::string& fileName, SourceLocation location, bool resolved,
            const optional<std::string> &callBy);
};

/*
Se realizó una implementación de inclusión de archivos algo primitiva pero eficaz
y directa.

Para evitar las inclusiones ciclicas debido a la naturaleza recursiva de la 
inclusión de archivos, se usó un Set de archivos para mantener un registro
de que archivos se han incluido
*/

class Preprocessor {
    private:
        File origin;
        bool fileExist(const std::ifstream &file);
        std::unordered_map<std::string, File> included;
        std::pair<std::string, std::size_t> getWord(const std::string &input_str, std::size_t index);
        bool contains(File f);

       
    public:
        Preprocessor(File origin);
        void markAsResolved(File file);
        std::string out;
        std::string includeFiles(File inputFile, int level);
        void diagnosticTool();
};

}; // namespace umbra

#endif