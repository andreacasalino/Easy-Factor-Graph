/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_QUASI_NEWTON_H
#define EFG_TRAINER_QUASI_NEWTON_H

#include <trainers/Commons.h>
#include <trainers/components/IterativeDescend.h>
#include <trainers/strategies/BFGS.h>
#include <trainers/strategies/BasicTrainSet.h>
#include <trainers/strategies/YundaSearcher.h>

namespace EFG::train {
template <typename TrainSetT = BasicTrainSet,
          typename LineSearcherT = YundaSearcher,
          typename HessianApproximatorT = BFGS>
class QuasiNewton : public IterativeDescend,
                    public TrainSetT,
                    public LineSearcherT,
                    public HessianApproximatorT {
  static_assert(std::is_base_of<BasicTrainSet, TrainSetT>::value,
                "TrainSetT should be a form of BasicTrainSet");
  static_assert(std::is_base_of<LineSearcher, LineSearcherT>::value,
                "LineSearcherT should be a form of LineSearcher");
  static_assert(
      std::is_base_of<HessianApproximator, HessianApproximatorT>::value,
      "HessianApproximatorT should be a form of HessianApproximator");

protected:
  inline void descend() override {
    // Vect direction = this->invHessianApprox * this->getGradient();
    Vect direction = this->getGradient();
    direction = this->invHessianApprox * direction;
    direction *= -1.f;
    this->minimize(direction);
  };

  void update() override {
    this->HessianApproximatorT::update();
    this->IterativeDescend::update();
  };

  void reset() override {
    this->HessianApproximatorT::reset();
    this->IterativeDescend::update();
  };
};
} // namespace EFG::train

#endif
