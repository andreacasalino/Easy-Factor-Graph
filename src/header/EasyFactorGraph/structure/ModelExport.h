/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO
#pragma once

#include <EasyFactorGraph/structure/ModelSeed.h>
#include <EasyFactorGraph/structure/Structure.h>

#include <filesystem>
#include <memory>

#include <nlohmann/json.hpp>

namespace EFG::structure {
structure::ModelSeed from_json(const nlohmann::json &json);

structure::ModelSeed from_file(const std::filesystem::path &file_name);

class ModelExport {
public:
  void to_json(nlohmann::json &json) const;

  void to_file(const std::filesystem::path &file_name) const;

protected:
  ModelExport() = default;

  void init(structure::StructurePtr context) { context_ = context; }

private:
  void exportVariables(nlohmann::json &json) const;
  void exportUnaryFactors(nlohmann::json &json) const;
  void exportBinaryFactors(nlohmann::json &json) const;

  structure::StructurePtr context_;
};
} // namespace EFG::structure
#endif
