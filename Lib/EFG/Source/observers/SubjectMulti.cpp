/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <observers/SubjectMulti.h>
#include "SubjectMultiCounter.h"

namespace EFG {
    SubjectMulti::SubjectMulti() 
        : Subject(std::make_shared<SubjectMultiCounter>()) {
    }
}
