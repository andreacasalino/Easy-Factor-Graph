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
        Matr(const std::size_t& size);  // zeros(size, size)
        Matr(const Vect& a, const Vect& b);

        Matr& operator+=(const Matr& m) const;

        Vect operator*(const Vect& v) const;
        Matr& operator*=(const Matr& m) const;
        Matr& operator*=(const float& m) const;

        void addIdentity();

    private:
        Vect buffer;
    };
}

#endif
#endif
