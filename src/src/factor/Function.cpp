/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/factor/Function.h>

#include <math.h>

namespace EFG::factor {
namespace {
std::size_t sizes_prod(const std::vector<std::size_t> &sizes) {
  std::size_t res = 1;
  for (auto v : sizes) {
    res *= v;
  }
  return res;
}

std::size_t compute_critical_size(std::size_t tot_size, float prctg) {
  auto res = std::floor(tot_size * prctg);
  return static_cast<std::size_t>(res);
}

static constexpr std::size_t MIN_CRITICAL = 6;
} // namespace

std::shared_ptr<const Function::Info> make_info(const categoric::Group &vars) {
  auto res = std::make_shared<Function::Info>();
  for (const auto &var : vars.getVariables()) {
    res->sizes.push_back(var->size());
  }
  res->totCombinations = sizes_prod(res->sizes);
  res->critical_size = std::max<std::size_t>(
      compute_critical_size(res->totCombinations, 0.5f), MIN_CRITICAL);
  return res;
}

Function::SparseContainer Function::makeSparseContainer() {
  return SparseContainer{MIN_CRITICAL, CombinationHasher{info}};
}

Function::Function(const categoric::Group &variables)
    : variables_{variables}, info{make_info(variables)},
      data_{makeSparseContainer()} {}

std::size_t Function::CombinationHasher::operator()(
    const std::vector<std::size_t> &comb) const {
  auto prod = info->totCombinations;
  std::size_t res = 0;
  for (std::size_t k = 0; k < info->sizes.size(); ++k) {
    prod /= info->sizes[k];
    res += comb[k] * prod;
  }
  return res;
}

void Function::set(const std::vector<std::size_t> &combination, float image) {
  Visitor<SparseContainer, DenseContainer>{
      [&combination, image = image, critical_size = info->critical_size,
       info = info, &data = data_](SparseContainer &c) {
        c[combination] = image;
        if (c.size() >= critical_size) {
          DenseContainer values;
          values.resize(info->totCombinations);
          for (auto &v : values) {
            v = 0;
          }
          CombinationHasher hasher{info};
          for (const auto &[comb, img] : c) {
            values[hasher(comb)] = img;
          }
          data = std::move(values);
        }
      },
      [&combination, image = image, info = info](DenseContainer &c) {
        c[CombinationHasher{info}(combination)] = image;
      }}
      .visit(data_);
}

float Function::findImage(const std::vector<std::size_t> &combination) const {
  float res;
  VisitorConst<SparseContainer, DenseContainer>{
      [&combination, &res](const SparseContainer &c) {
        if (auto it = c.find(combination); it != c.end()) {
          res = it->second;
        } else {
          res = 0;
        }
      },
      [&combination, &res, info = info](const DenseContainer &c) {
        CombinationHasher hasher{info};
        res = c[hasher(combination)];
      }}
      .visit(data_);
  return res;
}

float Function::findTransformed(
    const std::vector<std::size_t> &combination) const {
  float raw = findImage(combination);
  return transform(raw);
}

} // namespace EFG::factor
