#ifndef UMBRA_ERROR_MANAGER_H
#define UMBRA_ERROR_MANAGER_H

#include "CompilerError.h"
#include <memory>
#include <string>
#include <vector>

namespace umbra {

class ErrorManager {
  public:
    void addError(std::unique_ptr<CompilerError> error);
    bool hasErrors() const;
    std::string getErrorReport() const;
    size_t getErrorCount() const;
    void clear();
    const std::vector<std::unique_ptr<CompilerError>> &getErrors() const;
    void sortErrors();

  private:
    static const size_t MAX_ERRORS = 100;
    std::vector<std::unique_ptr<CompilerError>> errors;
};

} // namespace umbra

#endif // UMBRA_ERROR_MANAGER_H