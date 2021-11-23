/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_LINE_SEARCHER_H
#define EFG_TRAINER_LINE_SEARCHER_H

#include <trainers/Commons.h>
#include <trainers/components/ModelAware.h>

namespace EFG::train {
class LineSearcher : public virtual ModelAware {
protected:
  /**
   * @brief finds the values of the weight maximizing the likelihood, along the
   * passed direction
   * @param the direction along which to search the optimum
   */
  virtual void minimize(const Vect &direction) = 0;
};
} // namespace EFG::train

#endif
