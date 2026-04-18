#pragma once

#include <EasyFactorGraph/model/Graph.h>
#include <EasyFactorGraph/model/RandomField.h>

#include <span>
#include <unordered_map>

namespace EFG::test {
// TODO wire also to those test that build a simple binary factor model
class LineModel : public EFG::model::RandomField {
public:
  LineModel(categoric::VarStateSize var_size, float w);

  LineModel(categoric::VarStateSize var_size, const std::vector<float> &w);

private:
  static structure::ModelSeed make(categoric::VarStateSize var_size,
                                   std::span<const float> w);
};

class EvidenceTestModel
    : public EFG::model::Model<structure::QueryManager,
                               structure::EvidenceSetManager,
                               structure::EvidenceRemoveManager> {
public:
  const std::vector<std::size_t> uVars;
  const std::vector<std::size_t> mVars;
  const std::vector<std::size_t> lVars;

  EvidenceTestModel();

  bool clusterExists(const std::vector<std::size_t> &vars) const;

  bool allHiddenCluserExists() const;

  static constexpr inline std::size_t N = 4;

private:
  static structure::ModelSeed make();
};

class SimpleTree : public EFG::model::RandomField {
public:
  SimpleTree();

  std::size_t getVar(char symbol) const {
    static const std::unordered_map<char, std::size_t> table{
        {'A', 0}, {'B', 1}, {'C', 2}, {'D', 3}, {'E', 4},
    };
    return table.find(symbol)->second;
  }

  static const inline float alfa = 1.f;
  static const inline float beta = 2.f;
  static const inline float gamma = 1.f;
  static const inline float eps = 1.5f;

private:
  static structure::ModelSeed make();
};

class ComplexTree : public EFG::model::RandomField {
public:
  ComplexTree();

private:
  static structure::ModelSeed make();
};

class SimpleLoopy : public EFG::model::RandomField {
public:
  SimpleLoopy();

  std::size_t getVar(char symbol) const {
    static const std::unordered_map<char, std::size_t> table{
        {'A', 0}, {'B', 1}, {'C', 2}, {'D', 3}, {'E', 4},
    };
    return table.find(symbol)->second;
  }

  static const inline float w = 1.f;

private:
  static structure::ModelSeed make();
};

class ComplexLoopy : public EFG::model::RandomField {
public:
  ComplexLoopy();

private:
  static structure::ModelSeed make();
};

class MatrixLoopy : public EFG::model::RandomField {
public:
  MatrixLoopy(std::size_t size);

private:
  static structure::ModelSeed make(std::size_t size);
};

class ChainModel : public EFG::model::RandomField {
public:
  ChainModel(std::size_t len, float wXY, float wYY);

private:
  static structure::ModelSeed make(std::size_t len, float wXY, float wYY);
};
} // namespace EFG::test
