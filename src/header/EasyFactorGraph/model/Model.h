/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/ModelSeed.h>
#include <EasyFactorGraph/structure/Structure.h>

namespace EFG::model {
template <typename... Components> class Model : public Components... {
public:
  Model(structure::ModelSeed &&seed);

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;
  Model(Model &&) noexcept = delete;
  Model &operator=(Model &&) noexcept = delete;

private:
  template <typename ComponentFirst, typename... Rest>
  void init_components(structure::StructurePtr context) {
    this->ComponentFirst::init(context);
    if constexpr (0 < sizeof...(Rest)) {
      this->template init_components<Rest...>(context);
    }
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... Components>
Model<Components...>::Model(structure::ModelSeed &&seed) {
  auto models = std::make_shared<structure::Structure>(
      std::forward<structure::ModelSeed>(seed));
  this->template init_components<Components...>(models);
}
} // namespace EFG::model
