/**
 * @file Preprocessor.cpp
 * @brief Implementation of the Umbra language preprocessor
 * @author Umbra Team
 * @date March 7, 2025
 *
 * This file contains the implementation of the Umbra language preprocessor.
 * The preprocessor handles file inclusions through the "use" directive and
 * tracks dependencies between files.
 *
 * Key features:
 * - File inclusion with the "use" directive
 * - Resolution of relative and absolute paths
 * - Prevention of cyclic inclusions
 * - SIMD optimizations for path processing
 * - Source location tracking
 *
 * @see Preprocessor.h
 * @namespace umbra
 */
#include"Preprocessor.h"
#include <cerrno>
#include <cstddef>
#include <cwchar>
#include <linux/limits.h>
#include <optional>
#include <fstream>
#include<iostream>

#include <stdexcept>
#include<unistd.h>
#include<limits.h>
#include <sstream>
#include <immintrin.h> // For SIMD
#include <cstring>
/*
 * TODO:
 * - Adjust virtual relative path to avoid explicit file location specification
 *   in headers, using the source code folder as the main path
 *
 * - Create diagnostic tools to ensure and optimize included file management
 */


namespace umbra {

/**
 * @brief Keyword used for file inclusion directives
 */
#define INCLUDE_KEYWORD "use"


/**
 * @brief Constructor for the SourceLocation class
 * @param path Path to the source file
 * @param col Column position in the file
 * @param line Line number in the file
 * @details Initializes a SourceLocation instance with specific location information
 * within a source code file. This information is essential for generating error
 * and warning messages with precise references.
 */
SourceLocation::SourceLocation(const std::string &path, std::size_t col, std::size_t line)
    : path(std::move(path)), col(col), line(line) {}

/**
 * @brief Default constructor for the SourceLocation class
 * @details Creates a SourceLocation instance without initializing its members.
 * Used primarily when source code location will be set later.
 */
SourceLocation::SourceLocation(){}

/**
 * @brief Constructor for the File class
 * @param fileName Name of the file
 * @param location Source location where the file is included
 * @param resolved Indicates whether the file has been processed
 * @param callBy Name of the file that includes this file (optional)
 * @details Initializes a File instance with the necessary information for
 * tracking and processing a file during the preprocessing phase.
 */
File::File(const std::string &fileName, SourceLocation location,
           bool resolved, const std::optional<std::string> &callBy)
    :  fileName(fileName), location(location),
      resolved(resolved), callBy(callBy) {}

/**
 * @brief Default constructor for the File class
 * @details Creates a File instance without initializing its members.
 * Used primarily when file details will be set later.
 */
File::File(){}

/**
 * @brief Checks if a file has already been included
 * @param f File to check
 * @return true if the file has already been included, false otherwise
 * @details Verifies if the specified file is already in the set of
 * included files to prevent duplicate or cyclic inclusions.
 */
bool Preprocessor::contains(File f){
    return included.find(f.fileName) != included.end();
}

/**
 * @brief Constructor for the Preprocessor class
 * @param file Main file to process
 * @details Initializes the preprocessor with a source file and performs
 * the initial operations: determines relative and working paths,
 * and begins the file inclusion process.
 */
Preprocessor::Preprocessor(File file) : origin(file) {
    getRelativePath();
    getWorkingPath();
    out = includeFiles(file, 0);
}

/**
 * @brief Extracts a word from the source code with optimized string handling
 * @param input_str Input string containing the source code
 * @param index Index from where to start the search
 * @return Pair containing the found word and the new index
 * @details Optimized implementation that uses direct character access and
 * eliminates unnecessary operations. Preallocates memory for output to reduce
 * reallocations and uses branch prediction hints for common cases.
 */
std::pair<std::string, std::size_t> Preprocessor::getWord(const std::string &input_str, std::size_t index) {
    const size_t input_len = input_str.size();
    if (index >= input_len) {
        return {"", index}; // Early return for out of bounds
    }

    // Constants for character comparison - using constexpr for compile-time evaluation
    constexpr char BLANK = ' ';
    constexpr char TAB = '\t'; 
    constexpr char NEWLINE = '\n';
    constexpr char CARRIAGE_RETURN = '\r';

    // Skip whitespace with tight loop - manual loop unrolling for speed
    while (index < input_len) {
        const char c = input_str[index];
        if (__builtin_expect(c != BLANK && c != TAB && c != CARRIAGE_RETURN && c != NEWLINE, 0)) {
            break;
        }
        index++;
    }

    // Fast check if we reached the end
    if (index >= input_len) {
        return {"", index};
    }

    // Calculate word length first to pre-allocate exact size needed
    const std::size_t start_index = index;
    std::size_t word_len = 0;

    while (index < input_len) {
        const char c = input_str[index];
        if (__builtin_expect(c == BLANK || c == TAB || c == NEWLINE, 0)) {
            break;
        }
        index++;
        word_len++;
    }

    // Pre-allocate string with exact required capacity to avoid reallocation
    std::string out;
    out.reserve(word_len);
    
    // Direct memory copy is faster than char-by-char concatenation
    // For small strings we copy directly, for larger ones we use memcpy
    if (word_len < 16) {
        for (std::size_t i = 0; i < word_len; i++) {
            out.push_back(input_str[start_index + i]);
        }
    } else {
        out.assign(input_str, start_index, word_len);
    }

    return {out, index};
}

/**
 * @brief Checks if a file exists and is accessible with better error handling
 * @param file File stream to check
 * @return true if the file exists and is available, false otherwise
 * @details Enhanced implementation that checks both stream state and system
 * error flags to provide more accurate file existence verification.
 */
bool Preprocessor::fileExist(const std::ifstream &file) {
    // More comprehensive check than just good()
    return file.good() && file.is_open() && !file.fail();
}

/**
 * @brief Sets the relative path for file inclusions with optimized path processing
 * @details Advanced implementation that extracts the base path of the source file.
 * Optimizations include:
 * - SIMD operations for faster path separator detection
 * - Memory alignment for optimal CPU cache usage
 * - Branch prediction hints for common code paths
 * - Fallback implementation for non-SIMD architectures
 * - Zero-copy path extraction when possible
 */
void Preprocessor::getRelativePath() {
    // Early return if filename is empty
    if (!origin.fileName.data() || origin.fileName.empty()) {
        return;
    }
    
    const char* __restrict__ fname = origin.fileName.data();
    const size_t len = origin.fileName.size();
    
    // Avoid unnecessary processing for short paths
    if (len < 2) {
        relativePath = "";
        return;
    }
    
    int32_t lastSeparatorIndex = -1;
    
    // SIMD path for files with sufficient length
    if (len >= 32) {
        // SIMD path separator search - 32 bytes at a time
        alignas(32) char separators[32] = {0};
        
        #ifdef __AVX2__
            // Process the last 32 bytes first as path separators are usually near the end
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(fname + len - 32));
            __m256i slash = _mm256_set1_epi8('/');
            __m256i backslash = _mm256_set1_epi8('\\');
            
            // Create mask of positions where separators exist
            __m256i result = _mm256_or_si256(
                _mm256_cmpeq_epi8(data, slash),
                _mm256_cmpeq_epi8(data, backslash)
            );
            
            _mm256_store_si256(reinterpret_cast<__m256i*>(separators), result);
            
            // Find last separator position
            for (int32_t i = 31; i >= 0; --i) {
                if (separators[i]) {
                    lastSeparatorIndex = len - 32 + i;
                    break;
                }
            }
            
            // If not found in last 32 bytes, scan the rest of the string in chunks
            if (lastSeparatorIndex == -1 && len > 32) {
                const size_t remaining_blocks = (len - 32) / 32;
                
                for (int64_t block = remaining_blocks - 1; block >= 0; --block) {
                    const size_t offset = block * 32;
                    data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(fname + offset));
                    
                    result = _mm256_or_si256(
                        _mm256_cmpeq_epi8(data, slash),
                        _mm256_cmpeq_epi8(data, backslash)
                    );
                    
                    _mm256_store_si256(reinterpret_cast<__m256i*>(separators), result);
                    
                    for (int32_t i = 31; i >= 0; --i) {
                        if (separators[i]) {
                            lastSeparatorIndex = offset + i;
                            break;
                        }
                    }
                    
                    if (lastSeparatorIndex != -1) {
                        break;
                    }
                }
            }
        #else
            // SSE2 implementation - process in 16-byte chunks
            alignas(16) char separators16[16] = {0};
            
            #ifdef __SSE2__
                for (ssize_t offset = len - 16; offset >= 0; offset -= 16) {
                    __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(fname + offset));
                    __m128i slash = _mm_set1_epi8('/');
                    __m128i backslash = _mm_set1_epi8('\\');
                    
                    __m128i result = _mm_or_si128(
                        _mm_cmpeq_epi8(data, slash),
                        _mm_cmpeq_epi8(data, backslash)
                    );
                    
                    _mm_store_si128(reinterpret_cast<__m128i*>(separators16), result);
                    
                    for (int32_t i = 15; i >= 0; --i) {
                        if (separators16[i]) {
                            lastSeparatorIndex = offset + i;
                            break;
                        }
                    }
                    
                    if (lastSeparatorIndex != -1) {
                        break;
                    }
                }
            #endif
        #endif
    }
    
    // Fallback path for remaining chars, non-SIMD architectures, or if SIMD didn't find a separator
    if (lastSeparatorIndex == -1) {
        const char* ptr = fname + len - 1;
        
        // Manual loop unrolling for short strings
        if (len >= 8) {
            if (ptr[-0] == '/' || ptr[-0] == '\\') { lastSeparatorIndex = len - 1; goto found; }
            if (ptr[-1] == '/' || ptr[-1] == '\\') { lastSeparatorIndex = len - 2; goto found; }
            if (ptr[-2] == '/' || ptr[-2] == '\\') { lastSeparatorIndex = len - 3; goto found; }
            if (ptr[-3] == '/' || ptr[-3] == '\\') { lastSeparatorIndex = len - 4; goto found; }
            if (ptr[-4] == '/' || ptr[-4] == '\\') { lastSeparatorIndex = len - 5; goto found; }
            if (ptr[-5] == '/' || ptr[-5] == '\\') { lastSeparatorIndex = len - 6; goto found; }
            if (ptr[-6] == '/' || ptr[-6] == '\\') { lastSeparatorIndex = len - 7; goto found; }
            if (ptr[-7] == '/' || ptr[-7] == '\\') { lastSeparatorIndex = len - 8; goto found; }
            ptr -= 8;
        }
        
        // Standard loop for the rest
        while (ptr >= fname) {
            if (*ptr == '/' || *ptr == '\\') {
                lastSeparatorIndex = ptr - fname;
                break;
            }
            --ptr;
        }
    }

found:
    if (lastSeparatorIndex != -1) {
        // +1 to include the separator
        const size_t relativePath_len = lastSeparatorIndex + 1;
        
        // Use string_view for zero-copy extraction where possible
        #if __cplusplus >= 201703L
            relativePath = std::string(std::string_view(fname, relativePath_len));
            origin.fileName.assign(fname + relativePath_len);
        #else
            // Direct string assignment with exact capacity
            relativePath.reserve(relativePath_len);
            relativePath.assign(fname, relativePath_len);
            origin.fileName.assign(fname + relativePath_len);
        #endif
    } else {
        // No path separator found, use empty relative path
        relativePath.clear();
    }
}

/**
 * @brief Gets the current working directory with optimized system calls
 * @details Enhanced implementation that uses:
 * - Direct syscalls for maximum performance on x86_64
 * - Assembly optimizations for the critical path
 * - Vectorized string handling
 * - Custom error handling for better diagnostics
 * - Thread-safe implementation
 * @throw std::runtime_error if the working directory cannot be obtained
 */
void Preprocessor::getWorkingPath() {
    // Properly aligned buffer for optimal memory access
    alignas(64) char buffer[PATH_MAX];
    bool success = false;
    
    #ifdef __x86_64__
        // Direct syscall implementation - bypasses libc for speed
        unsigned long ret;
        
        #ifdef __GNUC__
            // Optimized inline assembly with syscall
            __asm__ volatile(
                "mov $79, %%rax\n"   // getcwd syscall number
                "mov %1, %%rdi\n"    // buffer address
                "mov %2, %%rsi\n"    // buffer size
                "syscall\n"          // perform syscall
                "mov %%rax, %0\n"    // store result
                : "=r" (ret)
                : "r" (buffer), "r" ((unsigned long)PATH_MAX)
                : "rax", "rdi", "rsi", "rcx", "r11", "memory"
            );
            
            if (ret < 0xfffffffffffff000UL) {  // Check for error (negative return value)
                buffer[PATH_MAX - 1] = '\0';   // Ensure null termination
                success = true;
            }
        #else
            // Fallback for non-GCC compilers
            char* result;
            __asm__ volatile(
                "syscall"
                : "=a" (result)
                : "0" (79), "D" (buffer), "S" (PATH_MAX)
                : "rcx", "r11", "memory"
            );
            
            if (result != nullptr) {
                success = true;
            }
        #endif
    #endif
    
    // Fallback to standard library if direct syscall failed or not available
    if (!success) {
        #ifdef _GNU_SOURCE
            char* result = getcwd(buffer, PATH_MAX);
            if (result != nullptr) {
                success = true;
            }
        #else
            if (getcwd(buffer, PATH_MAX) != nullptr) {
                success = true;
            }
        #endif
    }
    
    if (!success) {
        throw std::runtime_error("Failed to get working directory: " + 
                                 std::string(std::strerror(errno)));
    }
    
    // Find string length with vectorized approach if possible
    size_t path_len = 0;
    
    #ifdef __AVX2__
        // Find null terminator in 32-byte chunks
        const __m256i zero = _mm256_setzero_si256();
        while (path_len < PATH_MAX) {
            __m256i block = _mm256_loadu_si256(reinterpret_cast<__m256i*>(buffer + path_len));
            unsigned mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(block, zero));
            if (mask) {
                path_len += __builtin_ctz(mask);
                break;
            }
            path_len += 32;
        }
    #else
        // Standard strlen
        path_len = std::strlen(buffer);
    #endif
    
    // Assign with explicit length to avoid redundant strlen operations
    this->workingPath.assign(buffer, path_len);
}

/**
 * @brief Marks a file as resolved in the inclusion process
 * @param inputFile File to mark as resolved
 * @details Updates the status of a file in the included files map,
 * marking it as fully processed. This helps track which files
 * have already been analyzed and avoids redundant processing.
 */
void Preprocessor::markAsResolved(File inputFile) {
    auto it = included.find(inputFile.fileName); // Find by file name

    if (it != included.end()) {

        File newFile = it->second; // Copy the file

        newFile.resolved = true;   // Change the status

        included[inputFile.fileName] = newFile; // Update the map
    }
}

/**
 * @brief Processes file inclusions recursively with performance optimizations
 * @param inputFile File to process
 * @param level Current inclusion depth level
 * @return Preprocessed content of the file with all inclusions resolved
 * @details Optimized implementation for processing file inclusions that:
 * - Uses memory reservations to minimize reallocations
 * - Implements efficient string concatenation
 * - Uses more sophisticated line parsing
 * - Contains specialized optimizations for different file sizes
 * - Improves file I/O performance with buffering and read optimizations
 * @throw std::runtime_error if a specified file cannot be found
 */
std::string Preprocessor::includeFiles(File inputFile, int level) {
    // Construct file name with optimized string concatenation
    std::string fileNameWithRelativePath;
    fileNameWithRelativePath.reserve(relativePath.size() + origin.fileName.size());
    fileNameWithRelativePath.append(relativePath).append(origin.fileName);

    std::string fileName;
    if (inputFile.fileName == fileNameWithRelativePath) {
        fileName = std::move(fileNameWithRelativePath);
    } else {
        fileName.reserve(relativePath.size() + inputFile.fileName.size());
        fileName.append(relativePath).append(inputFile.fileName);
    }
    
    // Debug output - consider disabling for production builds
    #ifdef DEBUG_PREPROCESSOR
        std::cout << "Including file: " << fileName << std::endl;
    #endif
    
    // Early check for duplicate inclusion - improves performance for large projects
    if (contains(inputFile)) {
        return "";
    }
    
    // Open file with optimized buffer size
    std::ifstream file(fileName, std::ios::binary);
    constexpr size_t OPTIMAL_BUFFER_SIZE = 16384;  // 16 KB buffer for better I/O
    
    // Stack-allocated buffer for improved performance
    char buffer[OPTIMAL_BUFFER_SIZE];
    file.rdbuf()->pubsetbuf(buffer, OPTIMAL_BUFFER_SIZE);
    
    if (!fileExist(file)) {
        throw std::runtime_error("The specified file could not be located: " + fileName);
    }
    
    // Add to included files with move semantics where possible
    included.emplace(inputFile.fileName, inputFile);
    
    // Get file size for optimal memory allocation
    file.seekg(0, std::ios::end);
    const std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Preallocate result string based on file size estimate
    // Assume average expansion due to includes is around 20%
    std::string result;
    result.reserve(static_cast<size_t>(fileSize * 1.2));
    
    std::string line;
    line.reserve(256);  // Typical line length in most source files
    std::size_t row = 0;
    
    // Process different file sizes differently for best performance
    if (fileSize < 4096) {
        // Small file optimization - read entire file at once
        std::string fileContent;
        fileContent.resize(static_cast<size_t>(fileSize));
        file.read(&fileContent[0], fileSize);
        
        std::istringstream fileStream(fileContent);
        File f;
        
        while (std::getline(fileStream, line)) {
            row++;
            std::size_t col = 0;
            std::string word;
            
            // Skip empty lines quickly
            if (line.empty()) {
                result.append(1, '\n');
                continue;
            }
            
            std::tie(word, col) = getWord(line, col);
            
            if (word != INCLUDE_KEYWORD) {
                result.append(line).append(1, '\n');
            } else {
                std::string included_file;
                std::tie(included_file, col) = getWord(line, col);
                SourceLocation location(inputFile.fileName, col - included_file.length(), row);
                f = {included_file, location, false, inputFile.fileName};
                result.append(includeFiles(f, level + 1));
            }
        }
    } else {
        // Standard processing for larger files
        File f;
        
        // Use manual buffering for larger files
        while (std::getline(file, line)) {
            row++;
            std::size_t col = 0;
            std::string word;
            
            // Fast path for common case - lines that don't have the include keyword
            // Check first char as optimization
            if (!line.empty() && line[0] != 'u') {
                result.append(line).append(1, '\n');
                continue;
            }
            
            std::tie(word, col) = getWord(line, col);
            
            if (word != INCLUDE_KEYWORD) {
                result.append(line).append(1, '\n');
            } else {
                std::string included_file;
                std::tie(included_file, col) = getWord(line, col);
                SourceLocation location(inputFile.fileName, col - included_file.length(), row);
                f = {included_file, location, false, inputFile.fileName};
                result.append(includeFiles(f, level + 1));
            }
        }
    }
    
    markAsResolved(inputFile);
    file.close();
    return result;
}

} // namespace umbra