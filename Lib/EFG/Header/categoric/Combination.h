/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_CATEGORIC_COMBINATION_H
#define EFG_CATEGORIC_COMBINATION_H

#include <vector>

namespace EFG::categoric {
    /** 
     * @brief An immutable combination of discrete values
     */
    class Combination {
    public:
        /**
         * @brief A buffer of zeros with the passed size is created
         */
        Combination(std::size_t bufferSize);

        /**
         * @param the buffer to clone
         * @param the buffer size
         */
        Combination(const std::size_t* buffer, std::size_t bufferSize);

        Combination(const Combination& o);
        Combination& operator=(const Combination& o);

        ~Combination();

        /**
         * @brief compare two equally sized combination.
         * Examples of ordering:
         * <0,0,0> < <0,1,0>
         * <0,1> < <1,0>
         * @throw if the 2 combinations don't have the same number of values
         */
        bool operator<(const Combination& o) const;

        inline std::size_t size() const { return this->bufferSize; };
        inline const std::size_t* data() const { return this->buffer; };
        inline std::size_t* data() { return this->buffer; };

    private:
        const std::size_t bufferSize;
        std::size_t* buffer;
    };
}

#endif
