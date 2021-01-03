/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef _EFG_ERROR_RAISER_H_
#define _EFG_ERROR_RAISER_H_

#include <string>

namespace EFG {
    void raiseError(const std::string& objectName, const std::string& what);
}

#endif