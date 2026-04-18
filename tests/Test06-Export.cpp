#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/model/Graph.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/ModelExport.h>
#include <EasyFactorGraph/structure/SamplesImport.h>

#include <Utils.h>

#include <algorithm>
#include <ranges>
#include <set>

namespace EFG::test {
namespace {
class TmpFile {
public:
  TmpFile(std::string_view extension)
      : path_{std::filesystem::temp_directory_path() /
              misc::format("temp.{}", extension)} {}

  ~TmpFile() { std::filesystem::remove(path_); }

  const auto &getPath() const { return path_; }

private:
  std::filesystem::path path_;
};

bool are_same_sample(const misc::Samples &a, const misc::Samples &b) {
  auto it_a = a.makeIter();
  auto it_b = b.makeIter();
  while (true) {
    auto from_a = it_a.next();
    auto from_b = it_b.next();
    if (from_a.has_value() && from_b.has_value()) {
      std::span<const categoric::VarStateSize> from_a_val{from_a.value()};
      std::span<const categoric::VarStateSize> from_b_val{from_b.value()};
      if (!are_same(from_a_val, from_b_val)) {
        return false;
      }
    } else if (!from_a.has_value() && !from_b.has_value()) {
      break;
    } else {
      return false;
    }
  }
  return true;
}
} // namespace

TEST_CASE("Samples i/o", "[io][samples]") {
  std::size_t sample_size{4};

  misc::Samples samples{sample_size, 5};
  std::vector<categoric::VarStateSize> buffer;
  for (std::size_t k = 0; k < 13; ++k) {
    buffer.clear();
    for (std::size_t i = 0; i < sample_size; ++i) {
      buffer.push_back(static_cast<categoric::VarStateSize>(k));
    }
    samples.add(buffer);
  }

  TmpFile tmp_file{"bin"};
  structure::dump_train_set(samples, tmp_file.getPath());

  auto samples_imported = structure::load_train_set(tmp_file.getPath(), 3);

  CHECK(are_same_sample(samples, *samples_imported));
}

namespace {
template <typename M> class TestModel : public M {
public:
  static const inline float alfa = 0.5f;
  static const inline float beta = 0.7f;
  static const inline float gamma = 1.2f;

  static const inline std::vector<float> unary_shape =
      std::vector<float>{1.f, 2.f, 3.f};

  TestModel(structure::ModelSeed &&seed)
      : M{std::forward<structure::ModelSeed>(seed)} {}

  TestModel() : M{make_seed()} {}

  bool checkVariables() const {
    const structure::Structure &model = this->getStructure();

    for (auto label : std::vector<std::string>{"V0", "V1", "V2", "V3"}) {
      if (!model.named_vars_table.contains(label)) {
        return false;
      }
    }

    auto vars_rng =
        model.nodes | std::views::transform([](const auto &node) mutable {
          return std::make_pair(node.var_size,
                                node.evidence ==
                                    structure::Evidence::NOT_AN_EVIDENCE);
        });
    std::vector<std::pair<std::size_t, bool>> vars{vars_rng.begin(),
                                                   vars_rng.end()};

    return model.named_vars_table.size() == 4 &&
           vars == std::vector<std::pair<std::size_t, bool>>{
                       {3, true}, {3, true}, {3, true}, {3, true}, {3, true}};
  }

  bool checkUnaries() const {
    const structure::Structure &model = this->getStructure();

    bool V1_ok = check_unary(
        *std::get_if<factor::UnaryFactor>(&model.nodes[1].unary_factor));
    bool V2_ok = check_unary(*std::get_if<factor::UnaryFactorExponential>(
                                 &model.nodes[2].unary_factor),
                             alfa);
    bool V3_ok = check_unary(*std::get_if<factor::UnaryFactorExponential>(
                                 &model.nodes[3].unary_factor),
                             beta);

    return std::get_if<std::monostate>(&model.nodes[0].unary_factor) &&
           std::get_if<std::monostate>(&model.nodes[4].unary_factor) && V1_ok &&
           V2_ok && V3_ok;
  }

  bool checkBinaries() const {
    const structure::Structure &model = this->getStructure();

    if (model.binary_factors.size() != 8) {
      return false;
    }

    auto locate_factor_ = [&](std::size_t from, std::size_t to) {
      auto factor_index = model.locateBinaryFactor(from, to);
      return &model.binary_factors[factor_index].factor;
    };

    return check_binary(
               *std::get_if<factor::BinaryFactor>(locate_factor_(4, 0))) &&

           check_binary(
               *std::get_if<factor::BinaryFactor>(locate_factor_(4, 1))) &&

           check_binary(
               *std::get_if<factor::BinaryFactor>(locate_factor_(4, 2))) &&

           check_binary(
               *std::get_if<factor::BinaryFactor>(locate_factor_(4, 3)))

           &&

           check_binary(*std::get_if<factor::BinaryFactorExponential>(
                            locate_factor_(0, 1)),
                        alfa) &&

           check_binary(*std::get_if<factor::BinaryFactorExponential>(
                            locate_factor_(1, 2)),
                        beta) &&

           check_binary(*std::get_if<factor::BinaryFactorExponential>(
                            locate_factor_(2, 3)),
                        gamma) &&

           check_binary(*std::get_if<factor::BinaryFactorExponential>(
                            locate_factor_(3, 0)),
                        gamma);
  }

private:
  static structure::ModelSeed make_seed() {
    structure::ModelBuilder res;

    auto V0 = res.make_variable(
        3, structure::ModelBuilder::VariableDescription{false, "V0"});
    auto V1 = res.make_variable(
        3, structure::ModelBuilder::VariableDescription{false, "V1"});
    auto V2 = res.make_variable(
        3, structure::ModelBuilder::VariableDescription{false, "V2"});
    auto V3 = res.make_variable(
        3, structure::ModelBuilder::VariableDescription{false, "V3"});
    auto O = res.make_variable(3);

    ////////// add unary factors //////////

    res.add_unary_factor(factor::UnaryFactor{unary_shape}, V1);
    {
      auto factor = factor::UnaryFactorExponential{unary_shape};
      factor.trsfm.setWeight(alfa);
      res.add_unary_factor(std::move(factor), V2);
    }
    {
      auto factor = factor::UnaryFactorExponential{unary_shape};
      factor.trsfm.setWeight(beta);
      res.add_tunable_unary_factor(std::move(factor), V3);
    }

    ////////// add binary factors //////////

    auto add_binary_ = [&](std::size_t var_a, std::size_t var_b) {
      res.add_binary_factor(
          factor::make_simply_correlated<2, factor::NullTrasform>(3), var_a,
          var_b);
    };

    add_binary_(V0, O);
    add_binary_(V1, O);
    add_binary_(V2, O);
    add_binary_(V3, O);

    auto add_tunable_binary_ = [&](std::size_t var_a, std::size_t var_b,
                                   float w, bool tunable) {
      auto factor =
          factor::make_simply_correlated<2, factor::ExponentialTrasform>(3);
      factor.trsfm.setWeight(w);
      if (tunable) {
        res.add_tunable_binary_factor(std::move(factor), var_a, var_b);
      } else {
        res.add_binary_factor(std::move(factor), var_a, var_b);
      }
    };

    add_tunable_binary_(V0, V1, alfa, true);
    add_tunable_binary_(V1, V2, beta, true);
    add_tunable_binary_(V2, V3, gamma, false);
    add_tunable_binary_(V3, V0, gamma, false);

    return structure::ModelBuilder::build(std::move(res));
  }

  template <typename Transform>
  static bool check_unary(const factor::FactorT<1, Transform> &fctr) {
    std::vector<float> shape;
    fctr.template getValues<false>(shape);
    return shape == unary_shape;
  }

  static bool check_unary(const factor::UnaryFactorExponential &fctr, float w) {
    return check_unary(fctr) && fctr.trsfm.getWeight() == w;
  }

  template <typename Transform>
  static bool check_binary(const factor::FactorT<2, Transform> &fctr) {
    std::vector<float> shape;
    fctr.template getValues<false>(shape);
    return shape == std::vector<float>{1.f, 0, 0, 0, 1.f, 0, 0, 0, 1.f};
  }

  static bool check_binary(const factor::BinaryFactorExponential &fctr,
                           float w) {
    return check_binary(fctr) && fctr.trsfm.getWeight() == w;
  }
};
} // namespace

TEST_CASE("Model i/o", "[io][model]") {
  TmpFile tmp_file{"json"};

  SECTION("constant model") {
    TestModel<EFG::model::Graph> model;

    model.to_file(tmp_file.getPath());
    auto seed = structure::from_file(tmp_file.getPath());
    TestModel<EFG::model::Graph> model_imported{std::move(seed)};

    CHECK(model_imported.checkVariables());
    CHECK(model_imported.checkUnaries());
    CHECK(model_imported.checkBinaries());

    CHECK(model_imported.getStructure().tunability.order.empty());
  }

  SECTION("tunable model") {
    TestModel<EFG::model::RandomField> model;

    model.to_file(tmp_file.getPath());
    auto seed = structure::from_file(tmp_file.getPath());
    TestModel<EFG::model::RandomField> model_imported{std::move(seed)};

    CHECK(model_imported.checkVariables());
    CHECK(model_imported.checkUnaries());
    CHECK(model_imported.checkBinaries());

    std::vector<float> weights, weights_expected{
                                    model_imported.beta,
                                    model_imported.alfa,
                                    model_imported.beta,
                                };
    model_imported.getTunableWeights(weights);
    CHECK(weights == weights_expected);

    using TunableUnaryFactor = structure::Tunability::TunableUnaryFactor;
    using TunableBinaryFactor = structure::Tunability::TunableBinaryFactor;

    const auto &tunability = model_imported.getStructure().tunability.order;
    REQUIRE(tunability.size() == 3);
    CHECK(std::get<TunableUnaryFactor>(tunability[0]).var_index == 3);
    CHECK(std::get<TunableBinaryFactor>(tunability[1]).first_var_index == 0);
    CHECK(std::get<TunableBinaryFactor>(tunability[1]).second_var_index == 1);
    CHECK(std::get<TunableBinaryFactor>(tunability[2]).first_var_index == 1);
    CHECK(std::get<TunableBinaryFactor>(tunability[2]).second_var_index == 2);
  }
}
} // namespace EFG::test
