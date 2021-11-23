/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <trainers/Commons.h>
#include <trainers/components/HessianApproximator.h>

namespace EFG::train {
void HessianApproximator::update() {
  this->updateInvHessian(this->model->getWeights() - this->lastWeights,
                         this->getGradient() - this->lastGrad);
}

void HessianApproximator::reset() {
  std::size_t size = this->model->getWeights().size();
  this->invHessianApprox = Matr(size);
  this->invHessianApprox.addIdentity();
}
} // namespace EFG::train
