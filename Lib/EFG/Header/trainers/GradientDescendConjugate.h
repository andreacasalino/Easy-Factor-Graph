/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_CONJUGATE_GRADIENT_H
#define EFG_TRAINER_CONJUGATE_GRADIENT_H

#include <trainers/components/IterativeDescend.h>
#include <trainers/strategies/BasicTrainSet.h>
#include <trainers/strategies/YundaSearcher.h>

namespace EFG::train {
/**
 * @brief refer to <a href="linkURL">
 * https://www.caam.rice.edu/~zhang/caam454/pdf/cgsurvey.pdf </a> and the
 * possible strategies to compute beta
 */
class BetaStrategy : virtual public ModelAware {
protected:
  virtual float getBeta() const = 0;

  Vect lastDirection;
};

class FletcherReeves : public BetaStrategy {
protected:
  float getBeta() const override;
};

class PolakRibiere : public BetaStrategy {
protected:
  float getBeta() const override;
};

class HestenesStiefel : public BetaStrategy {
protected:
  float getBeta() const override;
};

class DaiYuan : public BetaStrategy {
protected:
  float getBeta() const override;
};

/**
 * @brief At every iteration the minimum along the direction of the conjugate
 * gradient method, <a href="linkURL">
 * https://www.caam.rice.edu/~zhang/caam454/pdf/cgsurvey.pdf </a> , is searched
 */
template <typename TrainSetT = BasicTrainSet,
          typename LineSearcherT = YundaSearcher,
          typename BetaStrategyT = FletcherReeves>
class GradientDescendConjugate : public IterativeDescend,
                                 public TrainSetT,
                                 public LineSearcherT,
                                 public BetaStrategyT {
  static_assert(std::is_base_of<BasicTrainSet, TrainSetT>::value,
                "TrainSetT should be a form of BasicTrainSet");
  static_assert(std::is_base_of<LineSearcher, LineSearcherT>::value,
                "LineSearcherT should be a form of LineSearcher");
  static_assert(std::is_base_of<BetaStrategy, BetaStrategyT>::value,
                "BetaStrategyT should be a form of BetaStrategy");

protected:
  void reset() override {
    this->IterativeDescend::reset();
    this->lastDirection = this->getGradient();
    this->lastDirection *= -1.f;
  }

  void descend() override {
    Vect direction = this->getGradient();
    direction *= -1.f;
    Vect correction = this->lastDirection;
    correction *= this->getBeta();
    direction += correction;
    this->minimize(direction);
    this->lastDirection = std::move(direction);
  };
};
} // namespace EFG::train

#endif
