/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/DistributionSetter.h>

namespace EFG::distribution {
void DistributionSetter::setImageRaw(const categoric::Combination &comb,
                                     const float &value) {
  if (value < 0.f) {
    throw Error("negative value is not possible");
  }
  const auto &vars = getVariables().getVariables();
  const auto size = vars.size();
  const auto &comb_data = comb.data();
  if (comb_data.size() != size) {
    throw Error{"Invalid combination"};
  }
  for (std::size_t k = 0; k < size; ++k) {
    if (vars[k]->size() <= comb_data[k]) {
      throw Error{"Invalid combination"};
    }
  }
  getCombinationsMap_()[comb] = value;
}

void DistributionSetter::setAllImagesRaw(const float &value) {
  if (value < 0.f) {
    throw Error("negative value is not possible");
  }
  clear();
  categoric::GroupRange range(getVariables());
  auto &map = getCombinationsMap_();
  for_each_combination(range,
                       [&map, &value](const categoric::Combination &comb) {
                         map[comb] = value;
                       });
}

void DistributionSetter::importCombiantionsFromFile(
    const std::string &file_name) {
  throw 0; // move to IO library
}
} // namespace EFG::distribution
