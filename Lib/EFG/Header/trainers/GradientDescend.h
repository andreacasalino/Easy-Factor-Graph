/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_GRADIENT_DESCEND_H
#define EFG_TRAINER_GRADIENT_DESCEND_H

#include <trainers/components/IterativeDescend.h>
#include <trainers/strategies/BasicTrainSet.h>
#include <trainers/strategies/YundaSearcher.h>

namespace EFG::train {
/**
 * @brief At every iteration the minimum along the gradient direction is
 * searched
 */
template <typename TrainSetT = BasicTrainSet,
          typename LineSearcherT = YundaSearcher>
class GradientDescend : public IterativeDescend,
                        public TrainSetT,
                        public LineSearcherT {
  static_assert(std::is_base_of<BasicTrainSet, TrainSetT>::value,
                "TrainSetT should be a form of BasicTrainSet");
  static_assert(std::is_base_of<LineSearcher, LineSearcherT>::value,
                "LineSearcherT should be a form of LineSearcher");

protected:
  inline void descend() override {
    Vect direction = this->getGradient();
    direction *= -1.f;
    this->minimize(direction);
  };
};
} // namespace EFG::train

#endif
