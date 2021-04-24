/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_ERROR_H
#define EFG_ERROR_H

#include <stdexcept>

namespace EFG {
    /** @brief A runtime error that can be raised when using any object in EFG::
	 */
    class Error : public std::runtime_error {
    public:
        explicit Error(const std::string& what);
    };
}

#endif