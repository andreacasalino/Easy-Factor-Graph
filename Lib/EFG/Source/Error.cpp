#include <Error.h>

namespace EFG {
    Error::Error(const std::string& objectName, const std::string& what)
        : Error(MergeArgs("EFG::", objectName, ": ", what)) {
    };

    Error::Error(const std::string& what)
        : std::runtime_error(what) {
    }

    void Error::mergeArgs(std::stringstream& stream, const std::string& a) { 
        stream << a; 
    };

    void Error::mergeArgs(std::stringstream& stream, const char* a) { 
        stream << a; 
    }
}