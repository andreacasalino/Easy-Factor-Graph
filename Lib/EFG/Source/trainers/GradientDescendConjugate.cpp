/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <trainers/GradientDescendConjugate.h>

namespace EFG::train {
float FletcherReeves::getBeta() const {
  Vect grad = this->getGradient();
  return dot(grad, grad) / dot(this->lastGrad, this->lastGrad);
}

float PolakRibiere::getBeta() const {
  Vect grad = this->getGradient();
  return dot(grad, grad - this->lastGrad) / dot(this->lastGrad, this->lastGrad);
}

float HestenesStiefel::getBeta() const {
  Vect grad = this->getGradient();
  Vect deltaGrad = grad - this->lastGrad;
  return dot(grad, deltaGrad) / dot(this->lastDirection, deltaGrad);
}

float DaiYuan::getBeta() const {
  Vect grad = this->getGradient();
  return dot(grad, grad) / dot(this->lastDirection, grad - this->lastGrad);
}
} // namespace EFG::train
