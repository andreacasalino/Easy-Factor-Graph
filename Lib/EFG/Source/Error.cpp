#include <Error.h>

namespace EFG {
    Error::Error(const std::string& what)
        : std::runtime_error(what) {
    }
}