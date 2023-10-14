#pragma once

#include <EasyFactorGraph/model/RandomField.h>

namespace EFG::test::library {
class SimpleTree : public EFG::model::RandomField {
public:
  SimpleTree();

  static const float alfa;
  static const float beta;
  static const float gamma;
  static const float eps;
};

class ComplexTree : public EFG::model::RandomField {
public:
  ComplexTree();
};

class SimpleLoopy : public EFG::model::RandomField {
public:
  SimpleLoopy();

  static const float w;
};

class ComplexLoopy : public EFG::model::RandomField {
public:
  ComplexLoopy();
};

class ScalableModel : public model::RandomField {
public:
  ScalableModel(std::size_t size, std::size_t var_size, const bool loopy);

  categoric::VariablePtr root() const;

  categoric::VariablePtr nonRoot() const;
};
} // namespace EFG::test::library
