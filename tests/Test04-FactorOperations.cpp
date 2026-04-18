#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/structure/BeliefManager.h>

#include <Utils.h>

#include <ranges>

namespace EFG::test {
using namespace categoric;
using namespace factor;
using namespace structure;

TEST_CASE("Unary factor merge factors", "[operations]") {
  UnaryFactorsMerger builder;
  builder.reset(3);

  SECTION("merge factors") {
    builder.template merge<true>(std::vector<float>{0.1f, 1.f, 1.f});
    builder.template merge<true>(std::vector<float>{1.f, 0.2f, 1.f});
    builder.template merge<true>(std::vector<float>{1.f, 1.f, 0.3f});

    std::vector<float> values{builder.getMerged().begin(),
                              builder.getMerged().end()};

    CHECK(test::almost_equal_it(
        values, std::vector<float>{0.1f / 0.3f, 0.2f / 0.3f, 0.3f / 0.3f},
        0.01f));
  }

  SECTION("merge exponential factors") {
    float a{1.f};
    float b{2.f};

    builder.template merge_factor<true>(make_unary_exp_factor(a, {0.1f, 0, 0}));
    builder.template merge_factor<true>(make_unary_exp_factor(b, {0, 0.2f, 0}));
    builder.normalize();

    std::vector<float> values{builder.getMerged().begin(),
                              builder.getMerged().end()};

    float d = expf(b * 0.2f);

    CHECK(test::almost_equal_it(
        values,
        std::vector<float>{expf(a * 0.1f) / d, expf(b * 0.2f) / d, 1.f / d},
        0.01f));
  }
}

TEST_CASE("Message passing", "[operations]") {
  std::array<float, 2> message_support;
  std::array<float, 3> sender_merged_unaries{1.f, 2.f, 1.f};
  auto edge = factor::BinaryFactor::from_compact_domain(
      Combination<2>{3, 2}, {4.f, 2.f, 2.f, 1.f, 1.f, 3.f});

  std::vector<float> message_values;

  structure::MessagePassingHandler mp;

  SECTION("SUM") {
    auto message = structure::make_passed_message<PropagationKind::SUM>(
        std::span<float>{message_support.begin(), message_support.end()},
        std::span<const float>{sender_merged_unaries.cbegin(),
                               sender_merged_unaries.cend()},
        false, edge);

    message.getValues<true>(message_values);
    std::vector<float> expected{9.f, 7.f};
    CHECK(almost_equal_it(message_values, expected, 0.01f));
  }

  SECTION("MAP") {
    auto message = structure::make_passed_message<PropagationKind::MAP>(
        std::span<float>{message_support.begin(), message_support.end()},
        std::span<const float>{sender_merged_unaries.cbegin(),
                               sender_merged_unaries.cend()},
        false, edge);

    message.getValues<true>(message_values);
    std::vector<float> expected{4.f, 3.f};
    CHECK(almost_equal_it(message_values, expected, 0.01f));
  }
}

TEST_CASE("Evidence creation", "[operations]") {
  std::array<float, 2> message_support;
  auto edge = factor::BinaryFactor::from_compact_domain(
      Combination<2>{3, 2}, {4.f, 2.f, 2.f, 1.f, 1.f, 3.f});

  SECTION("first var = 1") {
    auto message = factor::make_evidence_message(
        std::span<float>{message_support.begin(), message_support.end()}, edge,
        true, 1);

    std::vector<float> message_values;
    message.getValues<true>(message_values);
    std::vector<float> expected{2.f, 1.f};
    CHECK(almost_equal_it(message_values, expected, 0.01f));
  }

  SECTION("first var = 2") {
    auto message = factor::make_evidence_message(
        std::span<float>{message_support.begin(), message_support.end()}, edge,
        true, 2);

    std::vector<float> message_values;
    message.getValues<true>(message_values);
    std::vector<float> expected{1.f, 3.f};
    CHECK(almost_equal_it(message_values, expected, 0.01f));
  }
}

TEST_CASE("Make all same", "[operations]") {
  std::array<float, 3> support;
  auto factor = make_all_same(support, 2.f);

  std::vector<float> values;
  factor.getValues<true>(values);

  std::vector<float> expected{2.f, 2.f, 2.f};
  CHECK(almost_equal_it(values, expected, 0.01f));
}

TEST_CASE("Make indicator", "[operations]") {
  std::array<float, 3> support;
  auto factor = make_indicator(support, 1);

  std::vector<float> values;
  factor.getValues<true>(values);

  std::vector<float> expected{0, 1.f, 0};
  CHECK(almost_equal_it(values, expected, 0.01f));
}
} // namespace EFG::test
