#include <Error.h>

namespace EFG {
    Error::Error(const std::string& objectName, const std::string& what)
        : std::runtime_error(MergeArgs("EFG::", objectName, ": ", what)) {
    };

    void Error::mergeArgs(std::stringstream& stream, const std::string& a) { 
        stream << a; 
    };

    void Error::mergeArgs(std::stringstream& stream, const char* a) { 
        stream << a; 
    }
}