#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>

std::pair<std::string, size_t> getwrd(const std::string& input_str, size_t i) {

    const char BLANK = ' ';
    const char TAB = '\t';
    const char NEWLINE = '\n';

    std::string out;

    // Saltar espacios en blanco y tabulaciones iniciales
    while (i < input_str.size() && (input_str[i] == BLANK || input_str[i] == TAB)) {
        i++;
    }

    // Extraer la palabra
    while (i < input_str.size() && input_str[i] != BLANK && input_str[i] != TAB && input_str[i] != NEWLINE) {
        out += input_str[i];
        i++;
    }

    return {out, i};
}
//use file.def
void include(const std::string& file_name, int level = 0, int max_levels = 100) {
    const std::string incl_keyword = "use";
    if (level > max_levels) {
        std::cerr << "Error: includes nested too deeply. Max level is " << max_levels << ".\n";
        return;
    }
 
    std::ifstream infile(file_name);
    if (!infile) {
        std::cerr << "Error: No se pudo abrir el archivo '" << file_name << "'.\n";
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        size_t loc = 0;
        std::string word;
        std::tie(word, loc) = getwrd(line, loc);

        // Si la línea no contiene una inclusión, imprimirla
        if (word != incl_keyword) {
            std::cout << line << '\n';  
        } else {
            std::string included_file;
            std::tie(included_file, loc) = getwrd(line, loc);
            std::cout << "Incluyendo contenido de: " << included_file << '\n';

            include(included_file, level + 1);
        }
    }
}

// Ejemplo de uso
int main() {
    include("main.txt");
    return 0;
}
