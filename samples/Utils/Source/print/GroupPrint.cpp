/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <print/VariablePrint.h>
#include <print/GroupPrint.h>

std::ostream& operator<<(std::ostream& s, const EFG::categoric::Group& group) {
    auto it = group.getVariables().begin();
    s << **it;
    ++it;
    for(it; it != group.getVariables().end(); ++it) {
        s << ' ';
        s << **it;
    }
    return s;
}
