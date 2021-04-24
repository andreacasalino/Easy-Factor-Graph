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
    /** 
     * @brief A combination of discrete values
     */
    class Combination {
    public:
        /**
         * @param the values of the combination
         */
        explicit Combination(const std::vector<std::size_t>& values);

        /**
         * @param the buffer storing the values of the combination
         * @param the buffer size
         */
        Combination(const std::size_t* buffer, std::size_t bufferSize);

        Combination(const Combination& o);
        Combination& operator=(const Combination& o) = delete;

        inline std::size_t size() const { return this->combination.size(); };
        inline const std::size_t* data() const { return this->combination.data(); };

    private:        
        std::vector<std::size_t> combination;
    };

    /**
     * @brief compare two equally sized combination.
     * Examples of ordering:
     * <0,0,0> < <0,1,0>
     * <0,1> < <1,0> 
     * @throw if the 2 combinations don't have the same number of values
     */
    bool operator<(const Combination& a, const Combination& b);
}

#endif
