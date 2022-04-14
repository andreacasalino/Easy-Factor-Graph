#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/structure/components/PoolAware.h>

using namespace EFG;
using namespace EFG::strct;

TEST_CASE("testing Pool", "[pool]") {
  auto threads = GENERATE(1, 2, 4);

  SECTION("parallel for balance") {
    Pool pool(threads);

    std::vector<std::size_t> counters;
    for (std::size_t k = 0; k < threads; ++k) {
      counters.push_back(0);
    }

    const std::size_t times_x_thread = 5;
    Tasks tasks;
    const std::size_t tasks_size = times_x_thread * threads;
    tasks.reserve(tasks_size);
    for (std::size_t k = 0; k < tasks_size; ++k) {
      tasks.emplace_back(
          [&counters](const std::size_t th_id) { ++counters[th_id]; });
    }

    pool.parallelFor(tasks);
    for (const auto counter : counters) {
      CHECK(counter == times_x_thread);
    }
  }

  SECTION("pool reset") {
    class PoolContainerTest : public PoolAware {
    public:
      PoolContainerTest() = default;

      void setReset(const std::size_t threads) {
        setPoolSize(threads);
        resetPool();
      }
    };

    PoolContainerTest pool;
    for (std::size_t k = 0; k < 3; ++k) {
      pool.setReset(threads);
    }
  }
}
