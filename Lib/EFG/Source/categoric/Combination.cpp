/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <categoric/Combination.h>
#include <Error.h>
#include <cstring>

namespace EFG::categoric {
    Combination::Combination(std::size_t bufferSize)
        : bufferSize(bufferSize) {
        if (0 == this->bufferSize) {
            throw Error("size should be at least 1");
        }
        this->buffer = static_cast<std::size_t*>(malloc(sizeof(std::size_t) * bufferSize));
        for (std::size_t k = 0; k < this->bufferSize; ++k) {
            this->buffer[k] = 0;
        }
    }

    Combination::Combination(const std::size_t* buffer, std::size_t bufferSize)
        : bufferSize(bufferSize) {
        this->buffer = static_cast<std::size_t*>(malloc(sizeof(std::size_t) * bufferSize));
        std::memcpy(this->buffer, buffer, bufferSize);
    }

    Combination::Combination(const Combination& o)
        : Combination(o.buffer, o.bufferSize) {
    }

    Combination& Combination::operator=(const Combination& o) {
        if (this->bufferSize != o.bufferSize) {
            throw Error("can't copy a different sized combination");
        }
        std::memcpy(this->buffer, o.buffer, bufferSize);
        return *this;
    }

    Combination::~Combination() { 
        free(this->buffer); 
    };

    bool Combination::operator<(const Combination& o) const {
        if (this->bufferSize != o.bufferSize) {
            throw Error("uncomparable combinations");
        }
        for (std::size_t k = 0; k < this->bufferSize; ++k) {
            if (this->buffer[k] != o.buffer[k]) {
                return (this->buffer[k] < o.buffer[k]);
            }
        }
        return false;
    };
}
