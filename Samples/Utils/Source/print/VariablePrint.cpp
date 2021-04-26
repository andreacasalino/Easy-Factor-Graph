/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <print/VariablePrint.h>

std::ostream& operator<<(std::ostream& s, const EFG::categoric::Variable& variable) {
    s << '{' << variable.name() << ';' << variable.size() << '}';
    return s;
}
