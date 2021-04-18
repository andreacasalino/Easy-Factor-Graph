/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_COMBINATION_H
#define EFG_COMBINATION_H

#include <vector>

namespace EFG {
    class Combination {
    public:
        explicit Combination(const std::vector<std::size_t>& values);

        Combination(const std::size_t* buffer, std::size_t bufferSize);

        Combination(const Combination& o);
        Combination& operator=(const Combination& o) = delete;

        inline std::size_t size() const { return this->combination.size(); };
        inline const std::size_t* data() const { return this->combination.data(); };

    private:        
        std::vector<std::size_t> combination;
    };

    bool operator<(const Combination& a, const Combination& b);
}

#endif
