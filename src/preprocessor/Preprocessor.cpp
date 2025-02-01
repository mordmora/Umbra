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
#include <immintrin.h> // For SIMD
#include <cstring>
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
    out = includeFiles(file, 0);
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
    const char CARRIAGE_RETURN = '\r';
    std::string out;

    //Se salta los espacios en blanco

    while (index < input_str.size() && (input_str[index] == BLANK
        || input_str[index] == TAB || input_str[index] == CARRIAGE_RETURN || input_str[index] == NEWLINE))
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
    //out += NEWLINE;
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
void Preprocessor::getRelativePath() {
    if (!origin.fileName.data()) return;
    
    const char* __restrict__ fname = origin.fileName.data();
    const size_t len = origin.fileName.size();
    
    // SIMD path separator search
    alignas(32) char separators[32] = {0};
    __m256i separator_mask;
    
    #ifdef __AVX2__
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(fname + len - 32));
        __m256i slash = _mm256_set1_epi8('/');
        __m256i backslash = _mm256_set1_epi8('\\');
        
        __m256i result = _mm256_or_si256(
            _mm256_cmpeq_epi8(data, slash),
            _mm256_cmpeq_epi8(data, backslash)
        );
        
        _mm256_store_si256(reinterpret_cast<__m256i*>(separators), result);
    #endif
    
    int32_t lastSeparatorIndex = -1;
    
    // Fast path using SIMD results
    #ifdef __AVX2__
        for (int32_t i = 31; i >= 0; --i) {
            if (separators[i]) {
                lastSeparatorIndex = len - 32 + i;
                break;
            }
        }
    #endif
    
    // Fallback path for remaining chars or non-AVX2
    if (lastSeparatorIndex == -1) {
        const char* ptr = fname + len - 1;
        while (ptr >= fname) {
            if (*ptr == '/' || *ptr == '\\') {
                lastSeparatorIndex = ptr - fname;
                break;
            }
            --ptr;
        }
    }

    if (lastSeparatorIndex != -1) {
        char* temp = static_cast<char*>(__builtin_alloca(len + 1));
        
        // Optimize memory copies using intrinsics
        #ifdef __AVX2__
            size_t blocks = (lastSeparatorIndex + 1) / 32;
            char* dst = temp;
            const char* src = fname;
            
            for (size_t i = 0; i < blocks; ++i) {
                _mm256_store_si256(
                    reinterpret_cast<__m256i*>(dst),
                    _mm256_load_si256(reinterpret_cast<const __m256i*>(src))
                );
                dst += 32;
                src += 32;
            }
            
            memcpy(dst, src, (lastSeparatorIndex + 1) % 32);
        #else
            memcpy(temp, fname, lastSeparatorIndex + 1);
        #endif
        
        temp[lastSeparatorIndex + 1] = '\0';
        this->relativePath.assign(temp, lastSeparatorIndex + 1);
        this->origin.fileName.assign(fname + lastSeparatorIndex + 1);
    }
}

/*
 * Esta funcion obtiene la ruta de trabajo actual desde la cual se ejecuta el compilador
 * de Umbra, esto facilita el manejo de rutas
*/

void Preprocessor::getWorkingPath() {
    alignas(16) char buffer[PATH_MAX];
    
    #ifdef __x86_64__
        char* result;
        __asm__ volatile(
            "syscall"
            : "=a" (result)
            : "0" (79), // getcwd syscall number
              "D" (buffer),
              "S" (PATH_MAX)
            : "rcx", "r11", "memory"
        );
        
        if (result != nullptr) {
            this->workingPath = std::string(buffer);
            return;
        }
    #else
        if (getcwd(buffer, sizeof(buffer)) != nullptr) {
            this->workingPath = std::string(buffer);
            return;
        }
    #endif
    
    throw std::runtime_error("Failed to get working directory");
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

    std::string fileNameWithRelativePath = this->relativePath + this->origin.fileName;

    std::string fileName = inputFile.fileName == fileNameWithRelativePath ? fileNameWithRelativePath : this->relativePath+inputFile.fileName;
    std::cout << "Including file: " << fileName << std::endl;
    File f;
    std::ifstream file(fileName, std::ios::binary);
    if(!fileExist(file)){
        throw std::runtime_error("El archivo especificado no se pudo localizar");
    }
   
    //detectByteOrderMark(file);

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
        std::cout << "Word: " << word << std::endl;
        if(word != INCLUDE_KEYWORD){
            result += line + "\n"; 
        } else {
            std::cout << "Merging..." << std::endl;
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