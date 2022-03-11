/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::distribution {
class BasicEvaluator : public Evaluator {
public:
  BasicEvaluator() = default;

  float evaluate(const float &input) const final { return input; }
};

Factor::Factor(const categoric::Group &vars)
    : DistributionConcrete(std::make_shared<BasicEvaluator>(), vars) {}

namespace {
void check_all_same_size(const categoric::VariablesSoup &vars) {
  const std::size_t size = vars.front()->size();
  for (const auto &var : vars) {
    if (var->size() != size) {
      throw Error{"The passed variables don't have all the same size"};
    }
  }
}

std::vector<std::size_t> make_combination_buffer(const std::size_t size) {

  std::vector<std::size_t> result;
  result.reserve(size);
  for (std::size_t k = 0; k < size; ++k) {
    result.push_back(0);
  }
  return result;
}

void update_combination_buffer(std::vector<std::size_t> &buffer,
                               const std::size_t comb_value) {
  for (auto &val : buffer) {
    val = comb_value;
  }
}
} // namespace

Factor::Factor(const categoric::Group &vars, const UseSimpleCorrelation &)
    : Factor(vars) {
  check_all_same_size(vars.getVariables());
  auto buffer = make_combination_buffer(getVariables().getVariables().size());
  for (std::size_t k = 0; k < getVariables().getVariables().front()->size();
       ++k) {
    update_combination_buffer(buffer, k);
    auto buffer_copy = buffer;
    setImageRaw(categoric::Combination{std::move(buffer_copy)}, 1.f);
  }
}

Factor::Factor(const categoric::Group &vars, const UseSimpleAntiCorrelation &)
    : Factor(vars) {
  check_all_same_size(vars.getVariables());
  auto buffer = make_combination_buffer(getVariables().getVariables().size());
  setAllImagesRaw(1.f);
  for (std::size_t k = 0; k < getVariables().getVariables().front()->size();
       ++k) {
    auto buffer_copy = buffer;
    setImageRaw(categoric::Combination{std::move(buffer_copy)}, 0);
  }
}
} // namespace EFG::distribution
