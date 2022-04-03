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
