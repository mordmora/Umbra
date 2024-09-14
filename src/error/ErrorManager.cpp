#include "ErrorManager.h"
#include <algorithm>
#include <sstream>

namespace umbra {

void ErrorManager::addError(std::unique_ptr<CompilerError> error) {
    if (errors.size() < MAX_ERRORS) {
        errors.push_back(std::move(error));
    } else {
        throw std::runtime_error("Too many errors, aborted.");
    }
}

bool ErrorManager::hasErrors() const { return !errors.empty(); }

std::string ErrorManager::getErrorReport() const {
    std::ostringstream report;
    for (const auto &error : errors) {
        report << error->toString() << "\n";
    }
    return report.str();
}

size_t ErrorManager::getErrorCount() const { return errors.size(); }

void ErrorManager::clear() { errors.clear(); }

const std::vector<std::unique_ptr<CompilerError>> &ErrorManager::getErrors() const {
    return errors;
}

void ErrorManager::sortErrors() {
    std::sort(errors.begin(), errors.end(),
              [](const std::unique_ptr<CompilerError> &a, const std::unique_ptr<CompilerError> &b) {
                  if (a->getLine() != b->getLine()) {
                      return a->getLine() < b->getLine();
                  }
                  return a->getColumn() < b->getColumn();
              });
}

} // namespace umbra