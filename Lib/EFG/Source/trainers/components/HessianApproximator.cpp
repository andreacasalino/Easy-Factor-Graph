/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#include <trainers/components/HessianApproximator.h>
#include <trainers/Commons.h>

namespace EFG::train {
    void HessianApproximator::update() {
        this->updateInvHessian(this->model->getWeights() - this->lastWeights, this->getGradient() - this->lastGrad);
    }

    void HessianApproximator::reset() {
        std::size_t size = this->model->getWeights().size();
        this->invHessianApprox = Matr(size);
        this->invHessianApprox.addIdentity();
    }
}
#endif
