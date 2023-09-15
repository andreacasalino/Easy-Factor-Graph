/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Function.h>
#include <EasyFactorGraph/factor/ImageFinder.h>

namespace EFG::factor {
class ImageFinder;

class Immutable {
public:
  Immutable(const Immutable &o) = delete;
  Immutable &operator=(const Immutable &) = delete;
  Immutable(Immutable &&o) = delete;
  Immutable &operator=(Immutable &&) = delete;

  virtual ~Immutable() = default;

  const auto &function() const { return *function_; }

  ImageFinder makeFinder(const categoric::VariablesSoup &bigger_group) const;

  /**
   * @return the probabilities associated to each combination in the domain,
   * when assuming only the existance of this distribution. Such probabilities
   * are actually the normalized images. The order of returned values, refer to
   * the combinations that can be iterated by categoric::GroupRange on the
   * variables representing this distribution.
   */
  std::vector<float> getProbabilities() const;

protected:
  Immutable(FunctionPtr data);

private:
  FunctionPtr function_;
};

using ImmutablePtr = std::shared_ptr<Immutable>;
} // namespace EFG::factor
