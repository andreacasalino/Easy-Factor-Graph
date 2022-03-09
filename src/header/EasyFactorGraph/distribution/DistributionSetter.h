/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>

namespace EFG::distribution {
class DistributionSetter : virtual public Distribution {
public:
  /**
   * @brief sets the image of the passed combination.
   * In case the combination is currently not part of the distribution, it is
   * added with the passe raw image value.
   * @param the combination whose raw image must be set
   * @param the raw image value to assume
   * @throw passing a negative number for value
   */
  void setImageRaw(const categoric::Combination &comb, const float &value);

  /**
   * @brief sets the raw images of all the combinations equal to the passed
   * value
   * @throw passing a negative number for value
   */
  void setAllImagesRaw(const float &value);

  void clear() { getCombinationsMap().clear(); };
};
} // namespace EFG::distribution
