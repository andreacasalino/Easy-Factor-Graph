/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/factor/Transform.h>

namespace EFG::factor {
NullTrasform::NullTrasform() = default;

ExponentialTrasform::ExponentialTrasform() : w_{1.f} {}
} // namespace EFG::factor
