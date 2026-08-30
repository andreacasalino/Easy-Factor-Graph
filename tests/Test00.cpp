#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/misc/Samples.h>
#include <EasyFactorGraph/misc/WorkerPool.h>

#include <Utils.h>

namespace EFG::test {
using namespace misc;

TEST_CASE("generator materialization", "[misc]") {
  std::vector<int> values;

  SECTION("empty values") {
    CHECK(materialize<int>(GenFromBlock<int>{std::move(values)}).empty());
  }

  SECTION("some values") {
    values = {7, 4, 6, 8, 8, 2};

    auto values_back = materialize<int>(GenFromBlock<int>{values});
    CHECK(values == values_back);
  }
}

TEST_CASE("Samples iterator", "[misc]") {
  Samples samples{5};

  SECTION("Add invalid sample") { CHECK_THROWS(samples.add({1, 1})); }

  SECTION("add thn iterate") {
    std::vector<std::vector<categoric::VarStateSize>> values;
    for (std::size_t k = 0; k < 10; ++k) {
      auto &added = values.emplace_back();
      added.resize(5, k);
      samples.add(added);
    }

    std::vector<std::vector<categoric::VarStateSize>> values_back;
    auto it = samples.makeIter();
    while (true) {
      auto maybe_next = it.next();
      if (!maybe_next) {
        break;
      }
      values_back.emplace_back(std::vector<categoric::VarStateSize>{
          maybe_next->begin(), maybe_next->end()});
    }

    CHECK(values == values_back);
  }
}

TEST_CASE("WorkerPool compute", "[misc]") {
  auto threads = GENERATE(1, 2, 4);

  WorkerPool pool(threads);
  std::vector<std::size_t> counters;
  counters.resize(threads, 0);

  for (std::size_t k = 0; k < 3; ++k) {
    pool.compute([&](std::size_t th_id) { counters[th_id] += 1; });

    std::vector<std::size_t> counters_expected;
    counters_expected.resize(threads, k + 1);
    CHECK(counters == counters_expected);
  }
}
} // namespace EFG::test
