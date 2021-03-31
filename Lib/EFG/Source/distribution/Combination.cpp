/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Combination.h>
#include <cstring>

namespace EFG::distribution {
    Combination::~Combination() {
        delete[] this->data_;
    }

    Combination::Combination(const std::size_t* buffer, std::size_t size) {
        std::size_t* d = new std::size_t[size];
        std::memcpy(d, buffer, size);
        this->data_  = d;
    }

    Combination::Combination(const std::vector<std::size_t>& values) 
        : Combination(values.data(), values.size()) {
    }

    Combination::Combination(const Combination& o) 
        : Combination(o.data_, o.size_) {
    }
}
