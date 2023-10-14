/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "Utils.h"

#include <sstream>

namespace EFG::io {
std::pair<std::vector<std::size_t>, float>
parse_combination_image(const std::string &line) {
  std::stringstream buff{line};
  std::vector<std::string> tokens;
  {
    std::string token;
    while (buff >> token) {
      tokens.emplace_back(std::move(token));
    }
  }
  std::vector<std::size_t> comb;
  std::for_each(
      tokens.begin(), tokens.end() - 1, [&comb](const std::string &str) {
        comb.push_back(static_cast<std::size_t>(std::atoi(str.c_str())));
      });
  return std::make_pair(std::move(comb),
                        static_cast<float>(std::atof(tokens.back().c_str())));
}

void import_values(factor::Factor &recipient,
                   const std::filesystem::path &file_name) {
  useInStrem(file_name, [&](std::ifstream &stream) {
    recipient.clear();
    const std::size_t values_size =
        recipient.function().vars().getVariables().size();
    for_each_line(stream, [&values_size, &recipient](const std::string &line) {
      auto &&[comb, img] = parse_combination_image(line);
      if (comb.size() != values_size) {
        throw Error{"Invalid file content"};
      }
      recipient.set(comb, img);
    });
  });
}

void ImportHelper::importConst(const factor::ImmutablePtr &factor) {
  std::get<strct::FactorsConstInserter *>(model)->addConstFactor(factor);
}

void ImportHelper::importTunable(
    const std::shared_ptr<factor::FactorExponential> &factor,
    const std::optional<categoric::VariablesSet> &sharing_group) {
  if (auto ptr = std::get<train::FactorsTunableInserter *>(model); ptr) {
    if (sharing_group.has_value()) {
      this->cumulated.push_back(TunableCluster{factor, sharing_group.value()});
    } else {
      ptr->addTunableFactor(factor);
    }
    return;
  }
  std::get<strct::FactorsConstInserter *>(model)->addConstFactor(factor);
}

void ImportHelper::importCumulatedTunable() const {
  auto [constInserter, tunableInserter] = model;
  for (const auto &remaining : cumulated) {
    if (tunableInserter) {
      tunableInserter->addTunableFactor(remaining.factor,
                                        remaining.group_owning_w_to_share);
      continue;
    }
    constInserter->addConstFactor(remaining.factor);
  }
}

} // namespace EFG::io
