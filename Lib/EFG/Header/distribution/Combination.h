/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_COMBINATION_H
#define EFG_DISTRIBUTION_COMBINATION_H

#include <vector>

namespace EFG::distribution {
    class Combination {
    public:
        Combination(const std::vector<std::size_t>& values);
        Combination(const Combination& o);
        ~Combination();

        inline std::size_t size() const { return this->size_; };
        inline const std::size_t* data() const { return this->data_; };

    private:        
        Combination(const std::size_t* buffer, std::size_t size);

        std::size_t        size_;
        const std::size_t* data_;
    };
}

#endif
