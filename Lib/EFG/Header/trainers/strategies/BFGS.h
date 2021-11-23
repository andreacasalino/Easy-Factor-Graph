/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_BFGS_H
#define EFG_TRAINER_BFGS_H

#include <trainers/components/HessianApproximator.h>

namespace EFG::train {
/**
 * @brief The BFGS approach is used, as described here <a href="linkURL">
 * https://machinelearningmastery.com/bfgs-optimization-in-python/ </a>
 */
class BFGS : public HessianApproximator {
private:
  void updateInvHessian(const Vect &deltaWeight,
                        const Vect &deltaGrad) override;
};
} // namespace EFG::train

#endif
