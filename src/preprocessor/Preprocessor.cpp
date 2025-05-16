
#include "Preprocessor.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream> // Para depuración

namespace umbra {

Preprocessor::Preprocessor(const std::string& mainFilePath) {
    std::filesystem::path main_file_path_obj(mainFilePath);
    std::filesystem::path canonical_main_path;

    if (main_file_path_obj.empty()) {
        throw std::runtime_error("La ruta del archivo principal no puede estar vacía.");
    }

    try {

        if (main_file_path_obj.is_absolute()) {
            canonical_main_path = std::filesystem::weakly_canonical(main_file_path_obj);
        } else {
            canonical_main_path = std::filesystem::weakly_canonical(std::filesystem::current_path() / main_file_path_obj);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Error al procesar la ruta del archivo principal '" + mainFilePath + "': " + e.what());
    }

    if (!std::filesystem::is_regular_file(canonical_main_path)) {
        throw std::runtime_error("El archivo principal no existe o no es un archivo regular: " + canonical_main_path.string());
    }
    
    this->processedContent = processFile(canonical_main_path, 0);
}

std::string Preprocessor::getProcessedContent() const {
    return processedContent;
}

std::optional<std::string> Preprocessor::parseUseDirective(const std::string& line) {
    std::stringstream ss(line);
    std::string keyword;
    ss >> keyword; // Lee la primera palabra

    if (keyword == "use") { 
        std::string filePathWithQuotes;
        // Tomar el resto de la línea después de "use "
        size_t start_pos = line.find_first_not_of(" \t", keyword.length());
        if (start_pos == std::string::npos) return std::nullopt; // No hay nada después de "use"

        filePathWithQuotes = line.substr(start_pos);

        if (filePathWithQuotes.length() >= 2 && filePathWithQuotes.front() == '"' && filePathWithQuotes.back() == '"') {
            return filePathWithQuotes.substr(1, filePathWithQuotes.length() - 2);
        } else if (!filePathWithQuotes.empty() && filePathWithQuotes.find(' ') == std::string::npos) {
            return filePathWithQuotes;
        }
        std::cerr << "Warning: Malformed 'use' directive or unquoted path with spaces: " << line << std::endl;
        return std::nullopt;
    }
    return std::nullopt;
}

// Resuelve la ruta de inclusión y devuelve una ruta canónica
std::filesystem::path Preprocessor::resolveIncludePath(
    const std::filesystem::path& currentFileCanonicalPath, 
    const std::string& includeDirectivePathStr) {
    
    std::filesystem::path include_directive_path_obj(includeDirectivePathStr);
    std::filesystem::path resolved_path;

    if (include_directive_path_obj.is_absolute()) {
        resolved_path = include_directive_path_obj;
    } else {
        // La ruta es relativa al directorio del archivo actual
        std::filesystem::path current_file_directory = currentFileCanonicalPath.parent_path();
        resolved_path = current_file_directory / include_directive_path_obj;
    }
    
    try {

        return std::filesystem::weakly_canonical(resolved_path);
    } catch (const std::filesystem::filesystem_error& e) {

        std::cerr << "Advertencia al canonicalizar la ruta '" << resolved_path.string() << "': " << e.what() << ". Usando ruta normalizada léxicamente." << std::endl;
        return resolved_path.lexically_normal();
    }
}

std::string Preprocessor::processFile(const std::filesystem::path& currentFileCanonicalPath, int level) {
    if (level > MAX_INCLUDE_DEPTH) {
        throw std::runtime_error("Profundidad máxima de inclusión excedida, posible inclusión cíclica involucrando: " + currentFileCanonicalPath.string());
    }

    const std::string canonicalPathStr = currentFileCanonicalPath.string();

    if (includedFilesCanonicalPaths.count(canonicalPathStr)) {
        return ""; // Ya incluido
    }
    includedFilesCanonicalPaths.insert(canonicalPathStr);

    std::ifstream file_stream(currentFileCanonicalPath);
    if (!file_stream.is_open()) {

        throw std::runtime_error("No se pudo abrir el archivo para inclusión: " + canonicalPathStr);
    }


    std::stringstream content_buffer;
    std::string line;


    while (std::getline(file_stream, line)) {

        if (auto included_file_directive_path = parseUseDirective(line)) {
            std::filesystem::path next_file_to_include_canonical_path = 
                resolveIncludePath(currentFileCanonicalPath, *included_file_directive_path);
            
            content_buffer << processFile(next_file_to_include_canonical_path, level + 1);
        } else {
            content_buffer << line << '\n';
        }
    }

    return content_buffer.str();
}

} // namespace umbra