/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/misc/File.h>
#include <EasyFactorGraph/structure/ModelExport.h>

#include <fstream>
#include <ranges>
#include <unordered_set>

namespace EFG::structure {
namespace {
struct ConversionVariableDescription {
  static structure::ModelBuilder::VariableDescription
  from_json(const nlohmann::json &giver) {
    structure::ModelBuilder::VariableDescription recipient;
    if (giver.contains("isPermanentEvidence")) {
      recipient.isPermanentEvidence = giver["isPermanentEvidence"].get<bool>();
    }
    if (giver.contains("label")) {
      recipient.label = giver["label"].get<std::string>();
    }
    return recipient;
  }

  static void
  to_json(nlohmann::json &recipient,
          const structure::ModelBuilder::VariableDescription &desc) {
    recipient["isPermanentEvidence"] = desc.isPermanentEvidence;
    if (!desc.label.empty()) {
      recipient["label"] = desc.label;
    }
  }
};

std::pair<std::size_t, categoric::VarStateSize>
get_var(const nlohmann::json &giver, const ModelBuilder &model) {
  std::size_t index;
  if (giver.is_string()) {
    index = model.get_seed().named.at(giver.template get<std::string>());
  } else {
    index = giver.template get<std::size_t>();
  }
  return std::make_pair(index, model.get_seed().variables_sizes[index]);
};

bool is_tunable(const nlohmann::json &giver) {
  return giver.contains("tunable") ? giver["tunable"].get<bool>() : false;
}

struct ConversionUnary {
  static void from_json(const nlohmann::json &giver, ModelBuilder &model) {
    auto [var_index, var_size] = get_var(giver["var"], model);
    std::vector<float> values = giver["values"].get<std::vector<float>>();
    if (giver.contains("w")) {
      auto factor = factor::UnaryFactorExponential{values};
      factor.trsfm.setWeight(giver["w"].get<float>());
      if (is_tunable(giver)) {
        model.add_tunable_unary_factor(std::move(factor), var_index);
      } else {
        model.add_unary_factor(std::move(factor), var_index);
      }
    } else {
      auto factor = factor::UnaryFactor{values};
      model.add_unary_factor(std::move(factor), var_index);
    }
  }

  static void
  append_to_json(nlohmann::json &recipient,
                 const structure::EmptyOrFactorOrFactorExponential<1> &factor,
                 std::size_t var_index, bool is_tunable) {
    structure::use_factor_if(factor, [&](const auto &factor) {
      auto &added = recipient.emplace_back();
      added["var"] = var_index;
      std::vector<float> values;
      factor.template getValues<false>(values);
      added["values"] = values;
      if constexpr (std::is_same_v<decltype(factor),
                                   const factor::UnaryFactorExponential &>) {
        added["w"] = factor.trsfm.getWeight();
      }
      if (is_tunable) {
        added["tunable"] = true;
      }
    });
  }
};

struct ConversionSparseDomainPoint {
  static factor::SparseDomainPoint<2> from_json(const nlohmann::json &giver) {
    return std::make_pair(giver["c"].get<categoric::Combination<2>>(),
                          giver["v"].get<float>());
  }

  static void to_json(nlohmann::json &recipient,
                      const factor::SparseDomainPoint<2> &giver) {
    recipient["c"] = giver.first;
    recipient["v"] = giver.second;
  }
};

struct ConversionBinary {
  static void from_json(const nlohmann::json &giver, ModelBuilder &model) {
    auto [var_first_index, var_first_size] = get_var(giver["first_var"], model);
    auto [var_second_index, var_second_size] =
        get_var(giver["second_var"], model);

    auto add_exp_factor_ = [&](factor::BinaryFactorExponential &&res) {
      res.trsfm.setWeight(giver["w"].get<float>());
      if (is_tunable(giver)) {
        model.add_tunable_binary_factor(std::move(res), var_first_index,
                                        var_second_index);
      } else {
        model.add_binary_factor(std::move(res), var_first_index,
                                var_second_index);
      }
    };

    auto &values = giver["values"];

    if (values.is_string()) {
      if (var_first_size != var_second_size) {
        throw Error{"Can't have correlated or anticorrelated factor connecting "
                    "variables with "
                    "different sizes"};
      }

      const auto label = values.get<std::string>();
      if (label == "corr") {
        if (giver.contains("w")) {
          add_exp_factor_(
              factor::BinaryFactorExponential::from_sparse_domain_gen(
                  {var_first_size, var_second_size},
                  factor::SimplyCorrelatedDomainGen<2>{var_first_size}));
        } else {
          auto factor = factor::make_simply_correlated<2>(var_first_size);
          model.add_binary_factor(std::move(factor), var_first_index,
                                  var_second_index);
        }
      } else if (label == "anti") {
        if (giver.contains("w")) {
          auto values = misc::Intervals::from_gen(
              factor::SimplyAntiCorrelatedDomainGen<2>{var_first_size});
          add_exp_factor_(factor::BinaryFactorExponential{
              {var_first_size, var_second_size}, std::move(values)});
        } else {
          auto factor = factor::make_simply_anti_correlated<2>(var_first_size);
          model.add_binary_factor(std::move(factor), var_first_index,
                                  var_second_index);
        }
      } else {
        throw Error{"Unrecognized values kind for factor"};
      }

    }

    else {
      std::vector<factor::SparseDomainPoint<2>> combs;
      for (const auto &value : values) {
        combs.emplace_back(ConversionSparseDomainPoint::from_json(value));
      }

      if (giver.contains("w")) {
        add_exp_factor_(
            factor::BinaryFactorExponential::from_sparse_domain<true>(
                {var_first_size, var_second_size}, std::move(combs)));
      } else {
        auto factor = factor::BinaryFactor::from_sparse_domain<true>(
            {var_first_size, var_second_size}, std::move(combs));
        model.add_binary_factor(std::move(factor), var_first_index,
                                var_second_index);
      }
    }
  }

  static void to_json(nlohmann::json &recipient,
                      const structure::FactorOrFactorExponential<2> &factor,
                      std::size_t first_index, std::size_t second_index,
                      bool is_tunable) {
    std::visit(
        [&](const auto &factor) {
          recipient["first_var"] = first_index;
          recipient["second_var"] = second_index;
          auto &values = recipient["values"];
          values = nlohmann::json::array();
          factor.template forEachCombination<false>(
              [&](const categoric::Combination<2> &comb, float val) {
                ConversionSparseDomainPoint::to_json(values.emplace_back(),
                                                     std::make_pair(comb, val));
              });
          if constexpr (std::is_same_v<
                            decltype(factor),
                            const factor::BinaryFactorExponential &>) {
            recipient["w"] = factor.trsfm.getWeight();
          }
        },
        factor);
    recipient["tunable"] = is_tunable;
  }
};
} // namespace

structure::ModelSeed from_json(const nlohmann::json &json) {
  structure::ModelBuilder builder;
  for (const auto &var : json["variables"]) {
    auto sz = var["size"].get<categoric::VarStateSize>();
    auto desc = ConversionVariableDescription::from_json(var["description"]);
    builder.make_variable(sz, desc);
  }

  if (json.contains("unary_factors")) {
    for (const auto &unary : json["unary_factors"]) {
      ConversionUnary::from_json(unary, builder);
    }
  }

  if (json.contains("binary_factors")) {
    for (const auto &binary : json["binary_factors"]) {
      ConversionBinary::from_json(binary, builder);
    }
  }

  return structure::ModelBuilder::build(std::move(builder));
}

structure::ModelSeed from_file(const std::filesystem::path &file_name) {
  auto fd = misc::open<misc::FileMode::READ>(file_name);
  auto json = nlohmann::json::parse(*fd);
  return from_json(json);
}

void ModelExport::exportVariables(nlohmann::json &json) const {
  std::unordered_map<std::size_t, std::string_view> named_vars_inverted;
  for (const auto &[k, v] : context_->named_vars_table) {
    named_vars_inverted.emplace(v, std::string_view{k.data(), k.size()});
  }
  std::span<std::size_t> permanent_evidences{
      context_->permanent_evidences.data(),
      context_->permanent_evidences.size()};
  auto &variables = json["variables"];
  variables = nlohmann::json::array();
  for (std::size_t i = 0; i < context_->nodes.size(); ++i) {
    structure::ModelBuilder::VariableDescription desc;
    if (!permanent_evidences.empty() && permanent_evidences.front() == i) {
      desc.isPermanentEvidence = true;
      permanent_evidences = {permanent_evidences.begin() + 1,
                             permanent_evidences.end()};
    } else {
      desc.isPermanentEvidence = false;
    }

    if (auto it = named_vars_inverted.find(i);
        it != named_vars_inverted.end()) {
      desc.label = it->second;
    }

    auto &added = variables.emplace_back();
    ConversionVariableDescription::to_json(added["description"], desc);
    added["size"] = context_->nodes[i].var_size;
  }
}

void ModelExport::exportUnaryFactors(nlohmann::json &json) const {
  auto tunab_rng =
      context_->tunability.order | std::views::filter([](const auto &el) {
        return std::get_if<Tunability::TunableUnaryFactor>(&el);
      }) |
      std::views::transform([](const auto &el) {
        return std::get<Tunability::TunableUnaryFactor>(el).var_index;
      });
  std::unordered_set<std::size_t> tunab{tunab_rng.begin(), tunab_rng.end()};

  auto &factors = json["unary_factors"];
  factors = nlohmann::json::array();
  for (std::size_t i = 0; i < context_->nodes.size(); ++i) {
    ConversionUnary::append_to_json(factors, context_->nodes[i].unary_factor, i,
                                    tunab.contains(i));
  }
}

void ModelExport::exportBinaryFactors(nlohmann::json &json) const {
  auto tunab_rng =
      context_->tunability.order | std::views::filter([](const auto &el) {
        return std::get_if<Tunability::TunableBinaryFactor>(&el);
      }) |
      std::views::transform([](const auto &el) {
        auto &ref = std::get<Tunability::TunableBinaryFactor>(el);
        return std::make_pair(ref.first_var_index, ref.second_var_index);
      });
  std::unordered_set<std::pair<std::size_t, std::size_t>, PairHasher> tunab{
      tunab_rng.begin(), tunab_rng.end()};

  auto &factors = json["binary_factors"];
  factors = nlohmann::json::array();
  for (std::size_t i = 0; i < context_->nodes.size(); ++i) {
    auto rng = context_->nodes[i].incoming_messages |
               std::views::filter([](const auto &conn) {
                 return conn.factor_info.receiver_is_first_in_factor;
               }) |
               std::views::transform([](const auto &conn) {
                 return std::make_pair(conn.factor_info.sender_index,
                                       conn.factor_info.factor_index);
               });
    for (auto &&[second_index, factor_index] : rng) {
      bool is_tunab = tunab.contains(std::make_pair(i, second_index)) ||
                      tunab.contains(std::make_pair(second_index, i));
      ConversionBinary::to_json(factors.emplace_back(),
                                context_->binary_factors[factor_index].factor,
                                i, second_index, is_tunab);
    }
  }
}

void ModelExport::to_json(nlohmann::json &json) const {
  json = {};

  exportVariables(json);
  exportUnaryFactors(json);
  exportBinaryFactors(json);
}

void ModelExport::to_file(const std::filesystem::path &file_name) const {
  auto fd = misc::open<misc::FileMode::WRITE>(file_name);
  nlohmann::json json;
  to_json(json);
  *fd << json.dump(1);
}

} // namespace EFG::structure
#endif
