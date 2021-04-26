/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <categoric/Variable.h>
#include <Error.h>

namespace EFG::categoric {
    Variable::Variable(const std::size_t& size, const std::string& name)
        : Size(size)
        , Name(name) {
        if (name.size() == 0)  throw Error("empty name forbidden");
        if (size == 0) throw Error("null size forbidden");
    }
}
