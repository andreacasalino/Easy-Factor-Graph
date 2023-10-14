/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include <cmath>
#include <limits>
#include <algorithm>

namespace EFG::factor {
UnaryFactor::UnaryFactor(FunctionPtr data)
    : Factor{data}, variable{data->vars().getVariables().front()} {
  if (data->vars().getVariables().size() != 1) {
    throw Error{"Unary factor can refer only to unary group"};
  }
}

float UnaryFactor::diff(const UnaryFactor &o) const {
  auto this_prob = getProbabilities();
  auto o_prob = o.getProbabilities();
  float res = 0;
  for (std::size_t k = 0; k < this_prob.size(); ++k) {
    res += std::abs(this_prob[k] - o_prob[k]);
  }
  return res;
}

namespace {
class MergableFunction : public Function {
public:
  MergableFunction(const categoric::VariablePtr &var)
      : Function{categoric::Group{var}} {
    std::vector<float> imgs;
    for (std::size_t k = 0; k < var->size(); ++k) {
      imgs.push_back(1.f);
    }
    data_ = std::move(imgs);
    imgs_ = std::get_if<std::vector<float>>(&data_);
  }

  void merge(const Function &subject) {
    auto it = imgs_->begin();
    subject.forEachCombination<true>([&it](const auto &, float img) {
      *it *= img;
      ++it;
    });
  }

  void normalize() {
    float coeff = 1.f / *std::max_element(imgs_->begin(), imgs_->end());
    for (auto &val : *imgs_) {
      val *= coeff;
    }
  }

private:
  std::vector<float> *imgs_;
};
} // namespace

MergedUnaries::MergedUnaries(const categoric::VariablePtr &var)
    : UnaryFactor{std::make_shared<MergableFunction>(var)} {}

MergedUnaries::MergedUnaries(const std::vector<const Immutable *> &factors)
    : UnaryFactor(std::make_shared<MergableFunction>(
          factors.front()->function().vars().getVariables().front())) {
  for (const auto *factor : factors) {
    merge(*factor);
  }
  normalize();
}

void MergedUnaries::merge(const Immutable &to_merge) {
  const auto &vars = to_merge.function().vars().getVariables();
  if (vars.size() != 1) {
    throw Error{"Invalid factor"};
  }
  if (vars.front().get() != this->variable.get()) {
    throw Error{"Invalid factor"};
  }
  static_cast<MergableFunction &>(functionMutable()).merge(to_merge.function());
}

void MergedUnaries::normalize() {
  static_cast<MergableFunction &>(functionMutable()).normalize();
}

namespace {
categoric::VariablePtr get_other_var(const Immutable &binary_factor,
                                     const categoric::VariablePtr &var) {
  const auto &vars = binary_factor.function().vars().getVariables();
  if (2 != vars.size()) {
    throw Error{"invalid binary factor"};
  }
  return (vars.front() == var) ? vars.back() : vars.front();
}

void get_positions(const Immutable &binary_factor,
                   const categoric::VariablePtr &unary_factor_var,
                   std::size_t &unary_factor_var_pos,
                   std::size_t &other_var_pos) {
  unary_factor_var_pos = 0;
  other_var_pos = 1;
  if (binary_factor.function().vars().getVariables().back().get() ==
      unary_factor_var.get()) {
    std::swap(unary_factor_var_pos, other_var_pos);
  }
}
} // namespace

Evidence::Evidence(const Immutable &binary_factor,
                   const categoric::VariablePtr &evidence_var,
                   const std::size_t evidence)
    : UnaryFactor(std::make_shared<Function>(
          get_other_var(binary_factor, evidence_var))) {
  std::size_t pos_evidence;
  std::size_t pos_hidden;
  get_positions(binary_factor, getVariable(), pos_hidden, pos_evidence);
  auto &data = functionMutable();
  binary_factor.function().forEachCombination<true>(
      [&](const auto &comb, float img) {
        if (comb[pos_evidence] == evidence) {
          data.set(std::vector<std::size_t>{comb[pos_hidden]}, img);
        }
      });
}

Indicator::Indicator(const categoric::VariablePtr &var, std::size_t value)
    : UnaryFactor(std::make_shared<Function>(var)) {
  if (value >= var->size()) {
    throw Error{"Invalid indicator factor"};
  }
  functionMutable().set(std::vector<std::size_t>{value}, 1.f);
}

namespace {
template <typename ReducerT>
void fill_message(const UnaryFactor &merged_unaries,
                  const Immutable &binary_factor, Function &recipient) {
  std::size_t message_pos;
  std::size_t sender_pos;
  get_positions(binary_factor, merged_unaries.getVariable(), message_pos,
                sender_pos);
  std::size_t message_size =
      binary_factor.function().vars().getVariables()[message_pos]->size();
  for (std::size_t r = 0; r < message_size; ++r) {
    ReducerT reducer{};
    merged_unaries.function().forEachCombination<true>(
        [&](const auto &sender_comb, float sender_val) {
          std::vector<std::size_t> binary_factor_comb;
          binary_factor_comb.resize(2);
          binary_factor_comb[sender_pos] = sender_comb.front();
          binary_factor_comb[message_pos] = r;
          reducer.update(sender_val * binary_factor.function().findTransformed(
                                          binary_factor_comb));
        });
    recipient.set(std::vector<std::size_t>{r}, reducer.val);
  }
}
} // namespace

MessageSUM::MessageSUM(const UnaryFactor &merged_unaries,
                       const Immutable &binary_factor)
    : UnaryFactor(std::make_shared<Function>(
          get_other_var(binary_factor, merged_unaries.getVariable()))) {
  struct Reducer {
    void update(float v) { val += v; }
    float val = 0;
  };
  fill_message<Reducer>(merged_unaries, binary_factor, functionMutable());
}

MessageMAP::MessageMAP(const UnaryFactor &merged_unaries,
                       const Immutable &binary_factor)
    : UnaryFactor(std::make_shared<Function>(
          get_other_var(binary_factor, merged_unaries.getVariable()))) {
  struct Reducer {
    void update(float v) { val = std::max<float>(val, v); }
    float val = std::numeric_limits<float>::min();
  };
  fill_message<Reducer>(merged_unaries, binary_factor, functionMutable());
}
} // namespace EFG::factor
