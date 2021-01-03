/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <ErrorRaiser.h>
#include <Error.h>

namespace EFG {
    void raiseError(const std::string& objectName, const std::string& what) {
        throw Error(objectName, what);
    }
}