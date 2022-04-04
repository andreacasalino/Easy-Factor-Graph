/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/io/File.h>
#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

namespace EFG::io {
struct AdderPtrs {
  strct::FactorsAdder *as_factors_const_adder;
  train::FactorsTunableAdder *as_factors_tunable_adder;
};

template <typename Model> AdderPtrs getComponents(Model &model) {
  return AdderPtrs{dynamic_cast<strct::FactorsAdder *>(&model),
                   dynamic_cast<train::FactorsTunableAdder *>(&model)};
};

class Importer {
public:
  virtual ~Importer() = default;

protected:
  Importer() = default;

  virtual std::unordered_set<std::string>
  importComponents(const File &filePath, const AdderPtrs &subject) = 0;
};
} // namespace EFG::io
