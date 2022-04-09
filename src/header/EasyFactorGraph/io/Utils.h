/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/structure/components/StateAware.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

#include <fstream>

namespace EFG::io {
struct AwarePtrs {
  const strct::StateAware *as_structure_aware;
  const strct::FactorsAware *as_factors_const_aware;
  const train::FactorsTunableAware *as_factors_tunable_aware;
};

template <typename Model> AwarePtrs getAwareComponents(Model &model) {
  auto components =
      AwarePtrs{dynamic_cast<const strct::StateAware *>(&model),
                dynamic_cast<const strct::FactorsAware *>(&model),
                dynamic_cast<const train::FactorsTunableAware *>(&model)};
  if (nullptr == components.as_structure_aware) {
    throw Error{"A model should be at least StateAware"};
  }
  if (nullptr == components.as_factors_const_aware) {
    throw Error{"A model should be at least FactorsAware"};
  }
  return components;
};

struct AdderPtrs {
  strct::FactorsAdder *as_factors_const_adder;
  train::FactorsTunableAdder *as_factors_tunable_adder;
};

template <typename Model> AdderPtrs getAdderComponents(Model &model) {
  auto components =
      AdderPtrs{dynamic_cast<strct::FactorsAdder *>(&model),
                dynamic_cast<train::FactorsTunableAdder *>(&model)};
  if (nullptr == components.as_factors_const_adder) {
    throw Error{"A model should be at least FactorsAdder"};
  }
  return components;
};

using IStream = std::unique_ptr<std::ifstream>;
IStream make_in_stream(const std::string &file_name);

using OStream = std::unique_ptr<std::ofstream>;
OStream make_out_stream(const std::string &file_name);

template <typename Predicate>
void for_each_line(IStream &stream, const Predicate &pred) {
  std::string line;
  while (!stream->eof()) {
    line.clear();
    std::getline(*stream, line);
    pred(line);
  }
}

categoric::Combination
parse_combination(const std::vector<std::string> &values);
} // namespace EFG::io
