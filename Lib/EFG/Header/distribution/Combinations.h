/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_DISTRIBUTION_COMBINATIONS_H__
#define __EFG_DISTRIBUTION_COMBINATIONS_H__

#include <distribution/Group.h>
#include <util/Iterator.h>

namespace EFG::distr {

    class Combinations : public Group {
    public:
        Combinations(const std::vector<CategoricVariable*>& vars) : Group(vars) {};
        ~Combinations() { for (auto it = this->combinations.begin(); it != this->combinations.end(); ++it) free(*it); };

        inline std::size_t size() const { return this->combinations.size(); };

        template<typename Array>
        void                    add(const Array& comb);


        class  constIterator;
        inline constIterator    getIter() const;
    protected:
        // data
        std::list<size_t*>      combinations;
    };

    template<typename Array>
    void  Combinations::add(const Array& comb) {

        const std::vector<CategoricVariable*>& vars = this->GetVariables();
        std::size_t K = vars.size();
        std::size_t* clone = (size_t*)malloc(sizeof(size_t) * K);
        for (size_t k = 0; k < K; ++k) {
            clone[k] = comb[k];
            if (clone[k] >= vars[k]->size()) {
                free(clone);
                throw std::runtime_error("combination out of bounds");
            }
        }
        this->combinations.push_back(clone);

    };

    class Combinations::constIterator : public EFG::itr::TypedIterator<std::list<size_t*>::const_iterator> {
        friend class Combinations;
        constIterator(const Combinations& to_iterate)
            : EFG::itr::TypedIterator<std::list<size_t*>::const_iterator>(to_iterate.combinations.begin(), to_iterate.combinations.end()) {};
    public:
        inline const std::size_t* operator*() { return *this->cursor; };
    };
    Combinations::constIterator Combinations::getIter() const { return constIterator(*this); };

}

#endif