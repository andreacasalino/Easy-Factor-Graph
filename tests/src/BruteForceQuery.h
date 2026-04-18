/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/Samples.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/Structure.h>

#include <span>
#include <variant>

namespace EFG::test {
class BruteForce {
public:
  BruteForce(const structure::Structure &context);

  template <bool Logarithm>
  float getEnergy(std::span<const categoric::VarStateSize> comb) const;

  std::vector<float> getMarginals(std::size_t var_index) const;

  std::vector<float> getMarginals(std::size_t var_a_index,
                                  std::size_t var_b_index) const;

protected:
  const structure::Structure &context_;
  categoric::RangeVec comb_rng_;

  float getZ() const;

private:
  mutable std::optional<float> Z_proxy_;
};

struct BruteForceGradient : public BruteForce {
  BruteForceGradient(model::RandomField &model, std::size_t samples_len = 200);

  // BruteForceGradient(model::ConditionalRandomField &model);

  auto getSamples() const { return samples_; }

  std::vector<float> getGradientAt() const;

  float getLogLikelihoodAt() const;

private:
  std::shared_ptr<const misc::Samples> makeSamples(std::size_t len);

  float getAlfa(const structure::Tunability::TunableUnaryFactor &t) const;
  float getBeta(const structure::Tunability::TunableUnaryFactor &t) const;

  const factor::BinaryFactorExponential &find_binary_factor(
      const structure::Tunability::TunableBinaryFactor &t) const {
    const auto &incoming = context_.nodes[t.first_var_index].incoming_messages;
    auto it =
        std::find_if(incoming.begin(), incoming.end(), [&](const auto &conn) {
          return conn.factor_info.sender_index == t.second_var_index;
        });

    return std::get<factor::BinaryFactorExponential>(
        context_.binary_factors[it->factor_info.factor_index].factor);
  }
  float getAlfa(const structure::Tunability::TunableBinaryFactor &t) const;
  float getBeta(const structure::Tunability::TunableBinaryFactor &t) const;

  std::variant<model::RandomField *, model::ConditionalRandomField *> model_;
  std::shared_ptr<const misc::Samples> samples_;
};
} // namespace EFG::test
