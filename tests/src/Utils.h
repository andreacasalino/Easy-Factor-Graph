/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/structure/ModelSeed.h>
#include <EasyFactorGraph/structure/WorkerPoolManager.h>

#include <cmath>
#include <math.h>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

namespace EFG::test {
template <typename T>
bool are_same(std::span<const T> a, std::span<const T> b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t k = 0; k < a.size(); ++k) {
    if (a[k] != b[k]) {
      return false;
    }
  }
  return true;
}

template <typename T> bool almost_equal(T a, T b, T tollerance) {
  return std::abs(a - b) < tollerance;
}

template <typename IterableA, typename IterableB>
bool almost_equal_it(const IterableA &a, const IterableB &b, float tollerance) {
  if (a.size() != b.size()) {
    return false;
  }
  auto it_a = a.begin();
  auto it_b = b.begin();
  for (; it_a != a.end(); ++it_a, ++it_b) {
    if (!almost_equal(*it_a, *it_b, tollerance)) {
      return false;
    }
  }
  return true;
}

bool same_sign(float a, float b);

template <typename IterableA, typename IterableB>
bool same_sign_it(const IterableA &a, const IterableB &b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (!same_sign(a[i], b[i])) {
      return false;
    }
  }
  return true;
}

factor::UnaryFactorExponential
make_unary_exp_factor(float w, const std::vector<float> &domain);

void add_corr_expfactor(structure::ModelBuilder &res, float w,
                        std::size_t var_a, std::size_t var_b);

void add_indicator_expfactor(structure::ModelBuilder &res, float w,
                             std::size_t var);

template <typename M>
std::unique_ptr<structure::WorkerPoolManager::ScopedActivator>
activate_if_needed(M &model, std::size_t threads) {
  std::unique_ptr<structure::WorkerPoolManager::ScopedActivator> activator;
  if (1 < threads) {
    model.setWorkersPoolSize(threads);
    activator = model.activatePool();
  }
  return activator;
}
} // namespace EFG::test
