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
#include <variant>

namespace EFG::distribution {
class BasicEvaluator : public Evaluator {
public:
  BasicEvaluator() = default;

  float evaluate(const float &input) const final { return input; }
};

namespace {
static const std::shared_ptr<BasicEvaluator> BASIC_EVALUATOR =
    std::make_shared<BasicEvaluator>();
}

Factor::Factor(const Distribution &to_clone) : Factor(to_clone.getVariables()) {
  auto &comb_map = getCombinationsMap_();
  for (const auto &[comb, raw_val] : to_clone.getCombinationsMap()) {
    comb_map.emplace(comb, to_clone.evaluate(raw_val));
  }
}

Factor::Factor(const categoric::Group &vars)
    : DistributionConcrete(BASIC_EVALUATOR, vars) {}

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
    for (const auto &var : factor->getVariables().getVariablesSet()) {
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
  SparseTrivialFinder(const Distribution &subject)
      : distribution(distribution) {}

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
    : DistributionConcrete(BASIC_EVALUATOR, vars, map) {}

Factor::Factor(const Factor &o)
    : DistributionConcrete(
          BASIC_EVALUATOR, o.getVariables(),
          std::make_shared<CombinationRawValuesMap>(o.getCombinationsMap())) {}

Factor::Factor(Factor &&o) : DistributionConcrete(std::move(o)) {}
} // namespace EFG::distribution
