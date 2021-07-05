/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED

#include <trainers/GradientDescendConjugate.h>

namespace EFG::train {
    void GradientDescendConjugate::reset() {
        this->IterativeDescend::reset();
        this->lastDirection = this->getGradient();
    }

    void GradientDescendConjugate::descend() {
        Vect direction = this->getGradient();
        direction *= -1.f;
        direction += this->computeBeta() * this->lastDirection;
        this->minimize(direction);
        this->lastDirection = std::move(direction);
    }


}

#endif