/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <trainers/strategies/BFGS.h>

namespace EFG::train {
void BFGS::updateInvHessian(const Vect &deltaWeight, const Vect &deltaGrad) {
  float rho = 1.f / dot(deltaWeight, deltaGrad);
  Matr V(deltaGrad, deltaWeight);
  V *= -rho;
  V.addIdentity();
  Matr Vtrasp(deltaWeight, deltaGrad);
  Vtrasp *= -rho;
  Vtrasp.addIdentity();

  this->invHessianApprox = Vtrasp * this->invHessianApprox * V;
  Matr S(deltaWeight, deltaWeight);
  S *= rho;
  this->invHessianApprox += S;
}
} // namespace EFG::train
