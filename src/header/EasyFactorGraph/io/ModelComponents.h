/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/FactorsConstManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

#include <utility>

namespace EFG::io {
using Getters =
    std::tuple<const strct::StateAware *, const strct::FactorsConstGetter *,
               const train::FactorsTunableGetter *>;

template <typename ModelT> Getters castToGetters(ModelT &model) {
  auto components = std::make_tuple(
      dynamic_cast<const strct::StateAware *>(&model),
      dynamic_cast<const strct::FactorsConstGetter *>(&model),
      dynamic_cast<const train::FactorsTunableGetter *>(&model));
  if (nullptr == std::get<0>(components)) {
    throw Error{"A model should be at least StateAware"};
  }
  if (nullptr == std::get<1>(components)) {
    throw Error{"A model should be at least FactorsConstGetter"};
  }
  return components;
}

using Inserters =
    std::pair<strct::FactorsConstInserter *, train::FactorsTunableInserter *>;

template <typename Model> Inserters castToInserters(Model &model) {
  auto components =
      std::make_pair(dynamic_cast<strct::FactorsConstInserter *>(&model),
                     dynamic_cast<train::FactorsTunableInserter *>(&model));
  if (nullptr == std::get<0>(components)) {
    throw Error{"A model should be at least FactorsConstInserter"};
  }
  return components;
}
} // namespace EFG::io
