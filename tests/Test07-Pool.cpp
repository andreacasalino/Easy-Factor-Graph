#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/structure/bases/PoolAware.h>

#include "Utils.h"

namespace EFG::test {
using namespace strct;

TEST_CASE("testing Pool", "[pool]") {
  auto threads = GENERATE(1, 2, 4);

  SECTION("parallel for balance") {
    for (std::size_t k = 0; k < 5; ++k) {
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

TEST_CASE("testing Pool efficiency", "[pool]") {
  const std::size_t tasks_size = 20;
  Tasks tasks;
  for (std::size_t k = 0; k < tasks_size; ++k) {
    tasks.emplace_back([](const std::size_t) {
      std::this_thread::sleep_for(std::chrono::milliseconds{20});
    });
  }
  const std::size_t cycles = 5;

  auto measure_time =
      [&](const std::size_t threads) -> std::chrono::nanoseconds {
    Pool pool(threads);
    return test::measure_time([&]() {
      for (std::size_t k = 0; k < cycles; ++k) {
        pool.parallelFor(tasks);
      }
    });
  };

  auto single_thread_time = measure_time(1);
  auto multi_thread_time = measure_time(2);

  CHECK(static_cast<double>(multi_thread_time.count()) <
        static_cast<double>(0.7 * single_thread_time.count()));
}
} // namespace EFG::test
