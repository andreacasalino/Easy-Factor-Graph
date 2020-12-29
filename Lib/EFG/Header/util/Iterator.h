/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_ITERATOR_H__
#define __EFG_ITERATOR_H__

#include <functional>
#include <type_traits>

namespace EFG::itr {

    class Iterator {
    public:
        virtual Iterator& operator++() = 0;

        inline bool isNotAtEnd() const { return this->isActive; };

        Iterator(const Iterator&) = delete;
        void operator=(const Iterator&) = delete;
    protected:
        Iterator() : isActive(true) {};

        bool isActive;
    };



    class Decrementable : public Iterator {
    public:
        virtual Iterator& operator--() = 0;
    protected:
    };



    template<typename DerivedIter>
    void forEach(DerivedIter& toIter, const std::function<void(DerivedIter&)>& oper){
        if (!std::is_base_of<Iterator, DerivedIter>::value) return;
        while (toIter.isNotAtEnd()) {
            oper(toIter);
            ++toIter;
        }
    }


    template<typename iter>
    class TypedIterator : public itr::Decrementable {
    public:
        TypedIterator(const TypedIterator& o) : itr::Decrementable(o.cursor, o.cursor_end) {};
        void operator=(const TypedIterator& o) { this->cursor = o.cursor; this->c = o.cursor_end; };

        Iterator& operator++() final {
            ++this->cursor;
            if (this->cursor == this->cursor_end) this->isActive = false;
            return *this;
        };

        Iterator& operator--() final {
            --this->cursor;
            this->isActive = this->cursor != this->cursor_end;
            return *this;
        }
    protected:
        TypedIterator(const iter& c, const iter& end)
            : cursor(c)
            , cursor_end(end) {
            this->isActive = cursor != cursor_end;
        };

        iter   cursor;
        iter   cursor_end;
    };

}

#endif