/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/distribution/Factor.h>

#include <algorithm>
#include <mutex>
#include <variant>

namespace EFG::distribution {
class BasicEvaluator : public Evaluator {
public:
  BasicEvaluator() = default;

  float evaluate(const float &input) const final { return input; }
};

namespace {
static EvaluatorPtr BASIC_EVALUATOR;
static std::mutex BASIC_EVALUATOR_MTX;

const EvaluatorPtr &get_basic_evaluator() {
  std::scoped_lock lock(BASIC_EVALUATOR_MTX);
  if (nullptr == BASIC_EVALUATOR) {
    BASIC_EVALUATOR.reset(new BasicEvaluator());
  }
  return BASIC_EVALUATOR;
}
} // namespace

Factor::Factor(const Distribution &to_clone,
               const GenericCopyTag &GENERIC_COPY_TAG)
    : Factor(to_clone.getVariables()) {
  auto &comb_map = getCombinationsMap_();
  for (const auto &[comb, raw_val] : to_clone.getCombinationsMap()) {
    comb_map.emplace(comb, to_clone.evaluate(comb));
  }
}

Factor::Factor(const Factor &o) : Factor(o, GENERIC_COPY_TAG) {}

Factor::Factor(const categoric::Group &vars)
    : DistributionConcrete(get_basic_evaluator(), vars) {}

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
  if (1 == vars.size()) {
    throw Error{"Only 1 variable can't make a correlation"};
  }
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
  if (1 == vars.size()) {
    throw Error{"Only 1 variable can't make a correlation"};
  }
  check_all_same_size(vars.getVariables());
  auto buffer = make_combination_buffer(getVariables().getVariables().size());
  setAllImagesRaw(1.f);
  for (std::size_t k = 0; k < getVariables().getVariables().front()->size();
       ++k) {
    auto buffer_copy = buffer;
    for (auto &buffer_val : buffer_copy) {
      buffer_val = k;
    }
    setImageRaw(categoric::Combination{std::move(buffer_copy)}, 0);
  }
}

namespace {
categoric::Group
gather_variables(const std::vector<const Distribution *> &factors) {
  categoric::VariablesSet vars;
  for (const auto *factor : factors) {
    for (const auto &var : factor->getVariables().getVariables()) {
      auto vars_it = vars.find(var);
      if (vars_it == vars.end()) {
        vars.emplace(var);
      } else if (vars_it->get() != var.get()) {
        throw Error{
            var->name(),
            " appears multiple times, but in different variable instances"};
      }
    }
  }
  return categoric::Group{categoric::VariablesSoup{vars.begin(), vars.end()}};
}

class DenseTrivialFinder {
public:
  DenseTrivialFinder(const Distribution &subject)
      : evaluator(subject.getEvaluator()) {
    cursor = subject.getCombinationsMap().begin();
  }

  float get() const {
    auto cursor_prev = cursor;
    ++cursor;
    return evaluator.evaluate(cursor_prev->second);
  };

private:
  const Evaluator &evaluator;
  mutable CombinationRawValuesMap::const_iterator cursor;
};

class SparseTrivialFinder {
public:
  SparseTrivialFinder(const Distribution &subject) : distribution(subject) {}

  float get(const categoric::Combination &comb) const {
    return distribution.evaluate(comb);
  }

private:
  const Distribution &distribution;
};
} // namespace

Factor::Factor(const std::vector<const Distribution *> &factors)
    : Factor(gather_variables(factors)) {
  const auto &vars = getVariables().getVariables();
  categoric::GroupRange range(getVariables());
  const std::size_t range_size = getVariables().size();
  // fill the distribution
  auto &distribution = getCombinationsMap_();
  using CombinationFinderProxy =
      std::variant<DenseTrivialFinder, SparseTrivialFinder, CombinationFinder>;
  std::vector<CombinationFinderProxy> finders;
  finders.reserve(factors.size());
  for (const auto *factor : factors) {
    if (factor->getVariables().getVariables().size() == vars.size()) {
      if (factor->getCombinationsMap().size() == range_size) {
        finders.push_back(DenseTrivialFinder{*factor});
      } else {
        finders.push_back(SparseTrivialFinder{*factor});
      }
    } else {
      finders.push_back(factor->makeFinder(vars));
    }
  }
  struct CombinationFinderProxyVisitor {
    const categoric::Combination &comb;
    mutable float result = 0;

    void operator()(const DenseTrivialFinder &finder) const {
      result = finder.get();
    };

    void operator()(const SparseTrivialFinder &finder) const {
      result = finder.get(comb);
    };

    void operator()(const CombinationFinder &finder) const {
      result = finder.find(comb).value;
    };
  };
  categoric::for_each_combination(
      range, [&distribution, &finders](const categoric::Combination &comb) {
        float val = 1.f;
        std::find_if(finders.begin(), finders.end(),
                     [&comb, &val](const CombinationFinderProxy &finder) {
                       CombinationFinderProxyVisitor visitor{comb};
                       std::visit(visitor, finder);
                       if (0 == visitor.result) {
                         val = 0;
                         return true;
                       }
                       val *= visitor.result;
                       return false;
                     });
        if (val != 0) {
          distribution.emplace(comb, val);
        }
      });
}

Factor::Factor(const categoric::Group &vars,
               const CombinationRawValuesMapPtr &map)
    : DistributionConcrete(get_basic_evaluator(), vars, map) {}

namespace {
std::vector<std::size_t>
compute_new_positions(const categoric::VariablesSoup &old_order,
                      const categoric::VariablesSoup &new_order) {
  std::vector<std::size_t> result;
  result.reserve(old_order.size());
  for (const auto &var : old_order) {
    auto it = std::find(new_order.begin(), new_order.end(), var);
    result.push_back(std::distance(new_order.begin(), it));
  }
  return result;
}

std::vector<std::size_t>
get_permuted(const std::vector<std::size_t> &subject,
             const std::vector<std::size_t> &new_positions) {
  std::vector<std::size_t> result;
  result.resize(subject.size());
  for (std::size_t p = 0; p < subject.size(); ++p) {
    result[new_positions[p]] = subject[p];
  }
  return result;
}
} // namespace

Factor Factor::cloneWithPermutedGroup(const categoric::Group &new_order) const {
  if (new_order.getVariablesSet() != getVariables().getVariablesSet()) {
    throw Error{"Invalid new order. The new order should be a permutation of "
                "the factor to clone"};
  }
  Factor result(new_order);
  auto &result_combs = result.getCombinationsMap_();
  const auto new_positions = compute_new_positions(
      getVariables().getVariables(), result.getVariables().getVariables());
  for (const auto &[comb, val] : getCombinationsMap()) {
    result_combs.emplace(get_permuted(comb.data(), new_positions), val);
  }
  return result;
}
} // namespace EFG::distribution
