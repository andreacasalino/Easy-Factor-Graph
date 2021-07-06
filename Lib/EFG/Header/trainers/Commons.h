/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#ifndef EFG_TRAINER_COMMONS_H
#define EFG_TRAINER_COMMONS_H

#include <vector>

namespace EFG::train {
    typedef std::vector<float> Vect;

    Vect& operator-(Vect& v);

	Vect& operator+=(Vect& a, const Vect& b);

	Vect operator+(const Vect& a, const Vect& b);

	Vect& operator-=(Vect& a, const Vect& b);

	Vect operator-(const Vect& a, const Vect& b);

	float dot(const Vect& a, const Vect& b);

    Vect& operator*=(Vect& v, const float& c);

    Vect operator*(const Vect& v, const float& c);

    class Matr {
    public:
        Matr(const std::size_t size);  // zeros(size, size)
        Matr(const Vect& a, const Vect& b);

        Matr(Matr&&) = default;
        Matr& operator=(Matr&&) = default;

        void addIdentity();

        Matr& operator-();

        Matr& operator+=(const Matr& m);

        Vect operator*(const Vect& v) const;
        Matr operator*(const Matr& m) const;
        Matr& operator*=(const float& m);

    private:
        inline std::size_t getPos(const std::size_t row, const std::size_t col) const { return (row-1) * this->getSize() + col; };
        inline std::size_t getSize() const { return static_cast<std::size_t>(sqrt(static_cast<float>(this->buffer.size()))); };

        Vect buffer;
    };
}

#endif
#endif
