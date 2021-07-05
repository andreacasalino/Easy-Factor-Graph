/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#ifndef EFG_TRAINER_BFGS_H
#define EFG_TRAINER_BFGS_H

#include <trainers/components/HessianApproximator.h>

namespace EFG::train {
    class BFGS
        : public HessianApproximator {
    private:
        void updateInvHessian(const Vect& deltaWeight, const Vect& deltaGrad) override;
    };
}

#endif
#endif
