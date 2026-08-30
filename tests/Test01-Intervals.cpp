#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/misc/Intervals.h>

#include <Utils.h>

namespace EFG::test {
using namespace misc;
std::vector<std::pair<std::size_t, float>>
get_delimiters(const Intervals &subject) {
  const auto &block = subject.getDelimiters().get();
  return {block.begin(), block.end()};
}

TEST_CASE("testing from_value_by_value, no simplifications possible",
          "[intervals]") {
  std::vector<std::pair<std::size_t, float>> values_inputs{
      {{0, 2.f}, {2, 4.f}, {5, 3.f}}};

  SECTION("starting from 0") {
    auto values = Intervals::from_gen(
        GenFromBlock<std::pair<std::size_t, float>>{std::move(values_inputs)});

    std::vector<std::pair<std::size_t, float>> expected{
        {0, 2.f},
        {1, 0},
        {2, 4.f},
        {3, 0},
        {5, 3.f},
        {6, 0},
        {std::numeric_limits<std::size_t>::max(), 0}};

    CHECK(get_delimiters(values) == expected);
  }

  SECTION("not starting from 0") {
    values_inputs.erase(values_inputs.begin());

    auto values = Intervals::from_gen(
        GenFromBlock<std::pair<std::size_t, float>>{std::move(values_inputs)});

    std::vector<std::pair<std::size_t, float>> expected{
        {0, 0},   {2, 4.f}, {3, 0},
        {5, 3.f}, {6, 0},   {std::numeric_limits<std::size_t>::max(), 0}};

    CHECK(get_delimiters(values) == expected);
  }
}

TEST_CASE("testing from_value_by_value, with possible simplification",
          "[intervals]") {
  std::vector<std::pair<std::size_t, float>> values_inputs{
      {{2, 2.f}, {3, 2.f}, {7, 3.f}}};

  auto values = Intervals::from_gen(
      GenFromBlock<std::pair<std::size_t, float>>{std::move(values_inputs)});

  std::vector<std::pair<std::size_t, float>> expected{
      {0, 0},   {2, 2.f}, {4, 0},
      {7, 3.f}, {8, 0},   {std::numeric_limits<std::size_t>::max(), 0}};

  CHECK(get_delimiters(values) == expected);
}

TEST_CASE("testing from_value_by_value, with false simplification",
          "[intervals]") {
  std::vector<std::pair<std::size_t, float>> values_inputs{
      {{2, 2.f}, {3, 3.f}, {7, 3.f}}};

  auto values = Intervals::from_gen(
      GenFromBlock<std::pair<std::size_t, float>>{std::move(values_inputs)});

  std::vector<std::pair<std::size_t, float>> expected{
      {0, 0},
      {2, 2.f},
      {3, 3.f},
      {4, 0},
      {7, 3.f},
      {8, 0},
      {std::numeric_limits<std::size_t>::max(), 0}};

  CHECK(get_delimiters(values) == expected);
}

TEST_CASE("testing from_chunks", "[intervals]") {
  std::vector<Chunk> values_inputs{{2, 5, 5.f}, {10, 15, 2.f}};

  auto values =
      Intervals::from_gen(GenFromBlock<Chunk>{std::move(values_inputs)});

  std::vector<std::pair<std::size_t, float>> expected{
      {0, 0}, {2, 5.f}, {6, 0}, {10, 2.f}, {16, 0},
  };
}

TEST_CASE("testing get", "[intervals]") {
  std::vector<std::pair<std::size_t, float>> values_inputs{
      {{3, 2.f}, {6, 4.f}, {7, 3.f}}};

  auto values = Intervals::from_gen(
      GenFromBlock<std::pair<std::size_t, float>>{std::move(values_inputs)});

  CHECK(values.get(2) == 0);
  CHECK(values.get(3) == 2.f);
  CHECK(values.get(4) == 0);

  CHECK(values.get(5) == 0);
  CHECK(values.get(6) == 4.f);

  CHECK(values.get(7) == 3.f);
  CHECK(values.get(8) == 0);
  CHECK(values.get(10) == 0);
}

TEST_CASE("testing value iterator", "[intervals]") {
  std::vector<std::pair<std::size_t, float>> values_inputs{
      {{3, 2.f}, {6, 4.f}, {7, 3.f}}};

  auto values = Intervals::from_gen(
      GenFromBlock<std::pair<std::size_t, float>>{std::move(values_inputs)});

  std::vector<float> values_from_iter;
  auto iter = values.iter();
  for (std::size_t k = 0; k < 10; ++k) {
    values_from_iter.push_back(iter.next());
  }
  std::vector<float> values_from_iter_expected{0, 0,   0,   2.f, 0,
                                               0, 4.f, 3.f, 0,   0};

  CHECK(values_from_iter == values_from_iter_expected);
}

// negative tests

TEST_CASE("testing from_value_by_value with repettitions", "[intervals]") {
  std::vector<std::pair<std::size_t, float>> values_inputs{
      {{0, 2.f}, {2, 4.f}, {2, 3.f}}};

  CHECK_THROWS(Intervals::from_gen(
      GenFromBlock<std::pair<std::size_t, float>>{std::move(values_inputs)}));
}

TEST_CASE("testing from_chunks with collisions", "[intervals]") {
  SECTION("contained") {
    std::vector<Chunk> values_inputs{{0, 10, 5.f}, {5, 7, 2.f}};

    CHECK_THROWS(
        Intervals::from_gen(GenFromBlock<Chunk>{std::move(values_inputs)}));
  }

  SECTION("colliding") {
    std::vector<Chunk> values_inputs{{0, 10, 5.f}, {5, 15, 2.f}};

    CHECK_THROWS(
        Intervals::from_gen(GenFromBlock<Chunk>{std::move(values_inputs)}));
  }
}
} // namespace EFG::test
