#include <EasyFactorGraph/Error.h>

namespace EFG {
Error::Error(std::string what) : std::runtime_error(std::move(what)) {}
} // namespace EFG
