#include"Preprocessor.h"
#include <cerrno>
#include <cstddef>
#include <cwchar>
#include <linux/limits.h>
#include <optional>
#include <fstream>
#include<iostream>
#include <ostream>
#include <stdexcept>
#include<unistd.h>
#include<limits.h>
#include <sstream>
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


SourceLocation::SourceLocation(const std::string &path, std::size_t col, std::size_t line)
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
    getRelativePath();
    getWorkingPath();
    includeFiles(file, 0);
}

/*
 * Funcion auxiliar para encontrar una palabra en el archivo de entrada, es la encargada de
 * encontrar tanto la palabra reservada "use" como el nombre del archivo a incluir.
 *
 * Retorna una tupla en la cual contiene el contenido leido y el indice en el cual
 * termina
 */

std::pair<std::string, std::size_t> Preprocessor::getWord(const std::string &input_str, std::size_t index) {

    const char BLANK = ' ';
    const char TAB = '\t';
    const char NEWLINE = '\n';
    std::string out;

    //Se salta los espacios en blanco

    while (index < input_str.size() && (input_str[index] == BLANK
        || input_str[index] == TAB))
    {
        index++;
    }

    //Concatena los caracteres leidos en la variable out e incrementa el indice

    while (index < input_str.size() && input_str[index] != BLANK
        && input_str[index] != TAB && input_str[index] != NEWLINE)
    {
        out += input_str[index];
        index++;
    }

    return {out, index};
}

bool Preprocessor::fileExist(const std::ifstream &file){
    return file.good();
}

/*
* Esta funcion se encarga de establecer la ruta de usuario para mantener un tipo de ruta
* relativa virtual y facilitar la inclusion de archivos tomando como ruta raiz la ruta desde
* la cual es llamado el archivo principal del codigo fuente.
*
*/
void Preprocessor::getRelativePath(){

    std::string fileName = origin.fileName;
    std::string newFileName;
    int lastSeparatorIndex = -1;

    for(int i = fileName.size() - 1; i >= 0 ;--i){

        if(fileName[i] == '/' || fileName[i] == '\\'){

            lastSeparatorIndex = i;
            break;
        }
    }

    if(lastSeparatorIndex != -1){
        this->origin.fileName = fileName.substr(lastSeparatorIndex+1);
        this->relativePath = fileName.substr(0, lastSeparatorIndex + 1);
        return;
    }

}

/*
 * Esta funcion obtiene la ruta de trabajo actual desde la cual se ejecuta el compilador
 * de Umbra, esto facilita el manejo de rutas
*/

void Preprocessor::getWorkingPath(){

    char buffer[PATH_MAX];

    if(getcwd(buffer, sizeof(buffer)) != 0){
        this->workingPath = std::string(buffer);
        return;
    }

    int errorCode = errno;

    switch(errorCode){

        //EINVAL no puede suceder ya que arg size > 0
        //PATH_MAX contiene la terminacion en nulo,
        //por lo que ERANGE no debe ser retornado

        case EACCES:
            throw std::runtime_error("Acces denied.");

        case ENOMEM:
            //Muy poco probable que ocurra, pero por si acaso
            throw std::runtime_error("Insuficcient storage.");

        default: {
            std::ostringstream str;
            str << "Unrecognised error " << errorCode;
            throw std::runtime_error(str.str());
        }
    }
}

void Preprocessor::markAsResolved(File inputFile) {
    auto it = included.find(inputFile.fileName); // Buscar usando el nombre del archivo

    if (it != included.end()) {

        File newFile = it->second; // Copiar el archivo

        newFile.resolved = true;   // Cambiar el estado

        included[inputFile.fileName] = newFile; // Actualizar el map
    }
}


/*
* Esta funcion detecta y advierte al programador de que el codigo que esta ingresando
* soporta caracteres unicode, el uso de caracteres raros o iconos puede
* generar un comportamiento inesperado pero no es motivo para prohibir la lectura de
* un archivo de este tipo ya que es ineficiente validar la existencia de estos
* y es muy poco probable que sean usados por los usuarios objetivo
*/

void Preprocessor::detectByteOrderMark(std::ifstream &f){
    unsigned char byteOrderMark[4];
    f.read(reinterpret_cast<char*>(byteOrderMark), 4);

    //UTF-8 ok

    if(byteOrderMark[0] == 0xEF && byteOrderMark[1] == 0xBB && byteOrderMark[2] == 0xBF){

        return;
    }

    if(byteOrderMark[0] == 0xFE && byteOrderMark[1] == 0xFF){
        std::cout << "Warning__________________>>>> UTF-16 Little endian file detected" << std::endl;
        return;
    }

    if(byteOrderMark[0] == 0xFF && byteOrderMark[1] == 0xFE){
        std::cout << "Warning__________________>>>> UTF-16 Big endian file detected" << std::endl;
        return;
    }

    if(byteOrderMark[0] == 0x00 && byteOrderMark[1] == 0x00 && byteOrderMark[2] == 0xFE && byteOrderMark[3] == 0xFF){
        std::cout << "Warning__________________>>>> UTF-32 Little endian file detected" << std::endl;
        return;
    }

    if(byteOrderMark[0] == 0xFF && byteOrderMark[1] == 0xFE && byteOrderMark[2] == 0x00 && byteOrderMark[3] == 0x00){
        std::cout << "Warning_________________>>>> UTF-32 Big endiand file detected" << std::endl;
        return;
    }
    f.seekg(0);
}

std::string Preprocessor::includeFiles(File inputFile, int level){

    std::string fileNameWithRelativePath = this->relativePath + this->origin.fileName;

    std::string fileName = inputFile.fileName == fileNameWithRelativePath ? fileNameWithRelativePath : this->relativePath+inputFile.fileName;

    File f;
    std::ifstream file(fileName, std::ios::binary);
    if(!fileExist(file)){
        throw std::runtime_error("El archivo especificado no se pudo localizar");
    }

    detectByteOrderMark(file);

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

};

