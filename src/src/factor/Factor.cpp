/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
// #include <EasyFactorGraph/factor/CombinationFinder.h>
#include <EasyFactorGraph/factor/Factor.h>

namespace EFG::factor {
Factor::Factor(FunctionPtr data) : Immutable{data}, Mutable{data} {}

Factor::Factor(const categoric::Group &vars)
    : Factor{std::make_shared<Function>(vars)} {}

Factor::Factor(const Factor &o) : Factor{o.function().vars()} {
  functionMutable().cloneImages(o.function());
}

Factor::Factor(const Immutable &to_clone, CloneTrasformedImagesTag)
    : Factor{to_clone.function().vars()} {
  to_clone.function().forEachCombination<true>(
      [&recipient = functionMutable()](const auto &comb, float img) {
        recipient.set(comb, img);
      });
}

namespace {
void check_all_same_size(const categoric::VariablesSoup &vars) {
  const std::size_t size = vars.front()->size();
  for (const auto &var : vars) {
    if (var->size() != size) {
      throw Error{"The passed variables don't have all the same size"};
    }
  }
}

class SameValuesCombination : protected std::vector<std::size_t> {
public:
  SameValuesCombination(std::size_t size) {
    reserve(size);
    for (std::size_t k = 0; k < size; ++k) {
      push_back(0);
    }
  }

  std::vector<std::size_t> get() { return *this; }

  void next() {
    ++val;
    for (auto &v : *this) {
      v = val;
    }
  }

private:
  std::size_t val = 0;
};

class SimplyCorrelatedFunction : public Function {
public:
  SimplyCorrelatedFunction(const categoric::Group &variables)
      : Function{variables} {
    const auto &vars = variables.getVariables();
    check_all_same_size(vars);
    if (1 == vars.size()) {
      throw Error{"Only 1 variable can't make a correlation"};
    }
    auto imgs = makeSparseContainer();
    SameValuesCombination comb{variables.getVariables().size()};
    std::size_t each_var_size = variables.getVariables().front()->size();
    for (std::size_t k = 0; k < each_var_size; ++k, comb.next()) {
      imgs.emplace(comb.get(), 1.f);
    }
    data_ = std::move(imgs);
  }
};
} // namespace

Factor::Factor(const categoric::Group &vars, SimplyCorrelatedTag)
    : Factor{std::make_shared<SimplyCorrelatedFunction>(vars)} {}

namespace {
class SimplyAntiCorrelatedFunction : public Function {
public:
  SimplyAntiCorrelatedFunction(const categoric::Group &variables)
      : Function{variables} {
    const auto &vars = variables.getVariables();
    if (1 == vars.size()) {
      throw Error{"Only 1 variable can't make a correlation"};
    }
    check_all_same_size(vars);
    std::vector<float> imgs;
    imgs.reserve(info->totCombinations);
    for (std::size_t k = 0; k < info->totCombinations; ++k) {
      imgs.push_back(1.f);
    }
    CombinationHasher hasher{info};
    SameValuesCombination comb{variables.getVariables().size()};
    std::size_t each_var_size = variables.getVariables().front()->size();
    for (std::size_t k = 0; k < each_var_size; ++k, comb.next()) {
      imgs[hasher(comb.get())] = 0;
    }
    data_ = std::move(imgs);
  }
};
} // namespace

Factor::Factor(const categoric::Group &vars, SimplyAntiCorrelatedTag)
    : Factor{std::make_shared<SimplyAntiCorrelatedFunction>(vars)} {}

namespace {
categoric::Group
gather_variables(const std::vector<const Immutable *> &factors) {
  categoric::VariablesSet vars;
  for (const auto *factor : factors) {
    for (const auto &var : factor->function().vars().getVariables()) {
      auto vars_it = vars.find(var);
      if (vars_it == vars.end()) {
        vars.emplace(var);
      } else if (vars_it->get() != var.get()) {
        throw Error::make(
            var->name(),
            " appears multiple times, but in different variable instances");
      }
    }
  }
  return categoric::Group{categoric::VariablesSoup{vars.begin(), vars.end()}};
}
} // namespace

Factor::Factor(const std::vector<const Immutable *> &factors)
    : Factor(gather_variables(factors)) {
  if (factors.empty()) {
    throw Error{"Empty factors container"};
  }

  std::vector<const Immutable *> same_size_factors;
  std::vector<ImageFinder> finders;

  for (const auto *factor : factors) {
    if (factor->function().vars().getVariables() ==
        function().vars().getVariables()) {
      same_size_factors.push_back(factor);
    } else {
      finders.emplace_back(
          factor->makeFinder(function().vars().getVariables()));
    }
  }

  categoric::GroupRange range(function().vars());
  auto &recipient = functionMutable();
  categoric::for_each_combination(range, [&recipient, &same_size_factors,
                                          &finders](const auto &comb) {
    float val = 1.f;
    for (auto it = same_size_factors.begin();
         (it != same_size_factors.end()) && (val != 0); ++it) {
      val *= (*it)->function().findTransformed(comb);
    }
    for (auto it = finders.begin(); (it != finders.end()) && (val != 0); ++it) {
      val *= it->findTransformed(comb);
    }
    if (val != 0) {
      recipient.set(comb, val);
    }
  });
}

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
  if (new_order.getVariablesSet() != function().vars().getVariablesSet()) {
    throw Error{"Invalid new order. The new order should be a permutation of "
                "the factor to clone"};
  }
  auto data = std::make_shared<Function>(new_order);
  const auto new_positions = compute_new_positions(
      function().vars().getVariables(), data->vars().getVariables());
  function().forEachCombination<false>(
      [&recipient = *data, &new_positions](const auto &comb, float img) {
        recipient.set(get_permuted(comb, new_positions), img);
      });
  return data;
}
} // namespace EFG::factor
