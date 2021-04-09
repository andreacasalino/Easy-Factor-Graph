/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Combination.h>
#include <Error.h>
#include <cstring>

namespace EFG {
    Combination::Combination(const std::vector<std::size_t>& values) 
        : combination(values) {
    }

    Combination::Combination(const Combination& o) 
        : Combination(o.combination) {
    }

    Combination::Combination(const std::size_t* buffer, std::size_t bufferSize) {
        this->combination.reserve(bufferSize);
        std::memcpy(this->combination.data(), buffer, bufferSize);
    }

    bool operator<(const Combination& a, const Combination& b) {
        if (a.size() != b.size()) {
            throw Error("uncomparable combinations");
        }
        for (std::size_t k = 0; k < a.size(); ++k) {
            if (a.data()[k] != b.data()[k]) {
                return (a.data()[k] < b.data()[k]);
            }
        }
        return false;
    };
}
