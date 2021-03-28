/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <observers/SubjectUnique.h>
#include "SubjectUniqueCounter.h"

namespace EFG {
    SubjectUnique::SubjectUnique() 
        : Subject(std::make_shared<SubjectUniqueCounter>()) {
    }
}
