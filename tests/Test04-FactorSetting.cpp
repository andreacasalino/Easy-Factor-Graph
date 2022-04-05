#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/Factor.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

namespace {
bool set_image_raw(distribution::Factor &subject,
                   std::vector<std::size_t> &&to_add, const float value) {
  Combination comb(std::move(to_add));
  const std::size_t initial_map_size = subject.getCombinationsMap().size();
  subject.setImageRaw(comb, value);
  if (subject.evaluate(comb) != value) {
    return false;
  }
  return (initial_map_size + 1) == subject.getCombinationsMap().size();
}
} // namespace

TEST_CASE("operations on factor", "[factor]") {
  Factor factor(Group{
      {make_variable(3, "A"), make_variable(4, "B"), make_variable(2, "C")}});

  SECTION("set individual combination") {
    CHECK(set_image_raw(factor, {1, 2, 1}, 2.f));
    CHECK(set_image_raw(factor, {0, 0, 0}, 3.5f));
    CHECK(set_image_raw(factor, {2, 3, 0}, 1.f));
    CHECK(set_image_raw(factor, {1, 1, 1}, 0.5f));

    CHECK_THROWS_AS(set_image_raw(factor, {1, 0, 1}, -2.5f), Error);
  }

  SECTION("set all combinations at once") {
    auto value_to_set = GENERATE(2.f, 3.5f);
    factor.setAllImagesRaw(value_to_set);
    CHECK(factor.getCombinationsMap().size() == factor.getVariables().size());
    for (const auto &[comb, comb_val] : factor.getCombinationsMap()) {
      CHECK(value_to_set == comb_val);
    }

    CHECK_THROWS_AS(factor.setAllImagesRaw(-1.5f), Error);
  }
}

#include <EasyFactorGraph/io/FactorImporter.h>
#include <math.h>

TEST_CASE("import from file", "[factor]") {
  // 2,2,4
  categoric::VariablesSoup vars = {make_variable(2, "A"), make_variable(2, "B"),
                                   make_variable(4, "C")};
  distribution::Factor factor_ABC(categoric::Group{vars});
  const std::string file_name =
      std::string(TEST_FOLDER) + std::string("FactorDescription");

  io::import_values(factor_ABC, file_name);

  REQUIRE(factor_ABC.getCombinationsMap().size() == 4);
  CHECK(abs(factor_ABC.evaluate(Combination{{0, 1, 1}}) - 2.f) <= 0.01f);
  CHECK(abs(factor_ABC.evaluate(Combination{{0, 0, 0}}) - 3.f) <= 0.01f);
  CHECK(abs(factor_ABC.evaluate(Combination{{1, 1, 3}}) - 2.5f) <= 0.01f);
  CHECK(abs(factor_ABC.evaluate(Combination{{1, 0, 2}}) - 1.4f) <= 0.01f);
}

#include <EasyFactorGraph/categoric/GroupRange.h>

namespace {
bool all_equals_values(const Combination &comb) {
  const auto &data = comb.data();
  for (const auto &val : data) {
    if (val != data.front()) {
      return false;
    }
  }
  return true;
}
} // namespace

TEST_CASE("correlating factors", "[factor]") {
  Group group(VariablesSoup{make_variable(4, "A"), make_variable(4, "B"),
                            make_variable(4, "C")});

  SECTION("correlating factor") {
    Factor factor(group, USE_SIMPLE_CORRELATION_TAG);
    categoric::GroupRange range(factor.getVariables());
    for_each_combination(range, [&factor](const Combination &comb) {
      if (all_equals_values(comb)) {
        CHECK(1.f == factor.evaluate(comb));
      } else {
        CHECK(0 == factor.evaluate(comb));
      }
    });
  }

  SECTION("anti correlating factor") {
    Factor factor(group, USE_SIMPLE_ANTI_CORRELATION_TAG);
    categoric::GroupRange range(factor.getVariables());
    for_each_combination(range, [&factor](const Combination &comb) {
      if (all_equals_values(comb)) {
        CHECK(0 == factor.evaluate(comb));
      } else {
        CHECK(1.f == factor.evaluate(comb));
      }
    });
  }
}

TEST_CASE("merge factors", "[factor]") {
  const float val1 = 1.5f;
  const float val2 = 3.2f;

  auto varA = make_variable(2, "A");
  auto varB = make_variable(2, "B");
  auto varC = make_variable(2, "C");

  distribution::Factor distrAC(Group{{varA, varC}});
  distrAC.setAllImagesRaw(val1);
  distribution::Factor distrBC(Group{{varB, varC}});
  distrBC.setAllImagesRaw(val2);

  distribution::Factor distrABC(distrAC, distrBC);
  REQUIRE(3 == distrABC.getVariables().getVariables().size());

  const auto &distrABC_group = distrABC.getVariables().getVariablesSet();
  REQUIRE(distrABC_group.find(make_variable(2, "A")) != distrABC_group.end());
  REQUIRE(distrABC_group.find(make_variable(2, "B")) != distrABC_group.end());
  REQUIRE(distrABC_group.find(make_variable(2, "C")) != distrABC_group.end());

  GroupRange range(distrABC.getVariables());
  REQUIRE(distrABC.getVariables().size() == 2 * 2 * 2);
  for_each_combination(range, [&](const Combination &comb) {
    CHECK(distrABC.evaluate(comb) == val1 * val2);
  });
}
