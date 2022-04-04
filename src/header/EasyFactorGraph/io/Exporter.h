/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/io/File.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

namespace EFG::io {
struct AwarePtrs {
  strct::StateAware *as_structure_aware;
  strct::FactorsAware *as_factors_const_aware;
  train::FactorsTunableAware *as_factors_tunable_aware;
};

template <typename Model> AwarePtrs getComponents(Model &model) {
  return AwarePtrs{dynamic_cast<strct::StateAware *>(&model),
                   dynamic_cast<strct::FactorsAware *>(&model),
                   dynamic_cast<train::FactorsTunableAware *>(&model)};
};

class Exporter {
public:
  virtual ~Exporter() = default;

protected:
  Exporter() = default;

  virtual void exportComponents(const File &filePath,
                                const std::string &model_name,
                                const AwarePtrs &subject) = 0;
};
} // namespace EFG::io
