#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/factor/Function.h>

#include "Utils.h"

#include <cmath>

namespace EFG::test {
using Hasher = factor::Function::CombinationHasher;

using Combination = std::vector<std::size_t>;

TEST_CASE("Combination hasher", "[function]") {
  using Case = std::pair<Combination, std::size_t>;

  SECTION("single variable") {
    auto info = factor::make_info(make_group(std::vector<std::size_t>{4}));

    Hasher hasher{info};

    for (std::size_t k = 0; k < 4; ++k) {
      std::size_t index = hasher(std::vector<std::size_t>{k});
      CHECK(index == k);
    }
  }

  SECTION("multi variables") {
    auto info =
        factor::make_info(make_group(std::vector<std::size_t>{2, 4, 2}));

    Hasher hasher{info};

    std::vector<Case> cases;
    cases.emplace_back(std::make_pair(std::vector<std::size_t>{0, 0, 0}, 0));
    cases.emplace_back(std::make_pair(std::vector<std::size_t>{1, 3, 1},
                                      info->totCombinations - 1));
    cases.emplace_back(std::make_pair(std::vector<std::size_t>{0, 2, 0}, 4));
    cases.emplace_back(std::make_pair(std::vector<std::size_t>{0, 2, 1}, 5));
    cases.emplace_back(std::make_pair(std::vector<std::size_t>{1, 0, 0}, 8));
    cases.emplace_back(std::make_pair(std::vector<std::size_t>{1, 3, 0}, 14));
    cases.emplace_back(std::make_pair(std::vector<std::size_t>{1, 1, 1}, 11));

    for (const auto &[comb, index_expected] : cases) {
      std::size_t index = hasher(comb);
      CHECK(index == index_expected);
    }
  }
}

namespace {
enum class Case { NONE, SPARSE, DENSE };

class FunctionTestable : public factor::Function {
public:
  FunctionTestable()
      : factor::Function{make_group(std::vector<std::size_t>{2, 4, 2})} {}

  Case getCase() const {
    Case res = Case::NONE;
    VisitorConst<SparseContainer, DenseContainer>{
        [&res](const SparseContainer &c) { res = Case::SPARSE; },
        [&res](const DenseContainer &c) { res = Case::DENSE; }}
        .visit(data_);
    return res;
  }

  std::size_t size() const {
    std::size_t res;
    VisitorConst<SparseContainer, DenseContainer>{
        [&res](const SparseContainer &c) { res = c.size(); },
        [&res](const DenseContainer &c) { res = c.size(); }}
        .visit(data_);
    return res;
  }

  std::vector<float> getDenseImages() const {
    std::vector<float> res;
    VisitorConst<SparseContainer, DenseContainer>{
        [&res](const SparseContainer &c) {},
        [&res](const DenseContainer &c) { res = c; }}
        .visit(data_);
    return res;
  }
};

void addBelowCritical(FunctionTestable &subject) {
  subject.set(std::vector<std::size_t>{0, 1, 0}, 1.f);
  subject.set(std::vector<std::size_t>{1, 3, 0}, 2.f);
  subject.set(std::vector<std::size_t>{0, 2, 1}, 3.f);
}

void addAboveCritical(FunctionTestable &subject) {
  subject.set(std::vector<std::size_t>{0, 0, 0}, 1.f);
  subject.set(std::vector<std::size_t>{0, 0, 1}, 1.f);
  subject.set(std::vector<std::size_t>{0, 2, 0}, 1.f);
  subject.set(std::vector<std::size_t>{0, 2, 1}, 1.f);

  subject.set(std::vector<std::size_t>{1, 0, 0}, 1.f);
  subject.set(std::vector<std::size_t>{1, 0, 1}, 1.f);
  subject.set(std::vector<std::size_t>{1, 2, 0}, 1.f);
  subject.set(std::vector<std::size_t>{1, 2, 1}, 1.f);

  subject.set(std::vector<std::size_t>{1, 3, 1}, 1.f);
}

} // namespace

TEST_CASE("Function set method", "[function]") {
  FunctionTestable fnct;

  SECTION("empty after construction") {
    CHECK(fnct.size() == 0);
    CHECK(fnct.getCase() == Case::SPARSE);
  }

  SECTION("add remaining under critical") {
    addBelowCritical(fnct);
    CHECK(fnct.size() == 3);
    CHECK(fnct.getCase() == Case::SPARSE);
  }

  SECTION("add above critical") {
    addAboveCritical(fnct);
    CHECK(fnct.size() == 16);
    CHECK(fnct.getCase() == Case::DENSE);

    const auto images = fnct.getDenseImages();
    CHECK(images == std::vector<float>{1.f, 1.f, 0, 0, 1.f, 1.f, 0, 0, 1.f, 1.f,
                                       0, 0, 1.f, 1.f, 0, 1.f});
  }
}

TEST_CASE("Function find method", "[function]") {
  FunctionTestable fnct;

  fnct.set(std::vector<std::size_t>{0, 0, 0}, 1.f);
  fnct.set(std::vector<std::size_t>{1, 3, 0}, 1.f);

  auto comb = std::vector<std::size_t>{0, 2, 1};
  SECTION("set than find") {
    fnct.set(comb, 2.f);
    float img = fnct.findImage(comb);
    CHECK(img == 2.f);
  }

  SECTION("override value") {
    fnct.set(comb, 2.f);
    float img = fnct.findImage(comb);
    CHECK(img == 2.f);

    fnct.set(comb, 1.5f);
    img = fnct.findImage(comb);
    CHECK(img == 1.5f);
  }
}

namespace {
class FunctionWithTransform : public factor::Function {
public:
  template <typename Trsf>
  FunctionWithTransform(Trsf &&trsf)
      : factor::Function{make_group(std::vector<std::size_t>{2, 4, 2})},
        trsfm_{std::forward<Trsf>(trsf)} {}

protected:
  float transform(float input) const override { return trsfm_(input); }

private:
  std::function<float(float)> trsfm_;
};

} // namespace

TEST_CASE("Function transform", "[function]") {
  FunctionWithTransform fnct{[](float val) { return 2 * val; }};

  const auto comb = std::vector<std::size_t>{1, 3, 0};
  fnct.set(comb, 1.5f);
  CHECK(std::abs(fnct.findImage(comb) - 1.5f) < 0.001f);
  CHECK(std::abs(fnct.findTransformed(comb) - 3.f) < 0.001f);
}

TEST_CASE("Function for each method", "[function]") {
  FunctionTestable fnct;

  SECTION("iterate dense ") {
    SECTION("empty") {
      std::vector<float> images;
      fnct.forEachCombination<false>(
          [&images](const Combination &, float img) { images.push_back(img); });
      CHECK(images ==
            std::vector<float>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
    }

    SECTION("with some values") {
      addBelowCritical(fnct);

      std::vector<float> images;
      fnct.forEachCombination<false>(
          [&images](const Combination &, float img) { images.push_back(img); });
      CHECK(images == std::vector<float>{0, 0, 1.f, 0, 0, 3.f, 0, 0, 0, 0, 0, 0,
                                         0, 0, 2.f, 0});
    }
  }

  SECTION("iterate dense ") {
    addAboveCritical(fnct);

    std::vector<float> images;
    fnct.forEachCombination<false>(
        [&images](const Combination &, float img) { images.push_back(img); });
    CHECK(images == std::vector<float>{1.f, 1.f, 0, 0, 1.f, 1.f, 0, 0, 1.f, 1.f,
                                       0, 0, 1.f, 1.f, 0, 1.f});
  }
}

} // namespace EFG::test
