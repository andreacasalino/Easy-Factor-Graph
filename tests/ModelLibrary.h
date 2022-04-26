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
static const SimpleTree SIMPLE_TREE = SimpleTree{};

class ComplexTree : public EFG::model::RandomField {
public:
  ComplexTree();
};
static const ComplexTree COMPLEX_TREE = ComplexTree{};

class SimpleLoopy : public EFG::model::RandomField {
public:
  SimpleLoopy();

  static const float w;
};
static const SimpleLoopy SIMPLE_LOOPY = SimpleLoopy{};

class ComplexLoopy : public EFG::model::RandomField {
public:
  ComplexLoopy();
};
static const ComplexLoopy COMPLEX_LOOPY = ComplexLoopy{};
} // namespace EFG::test::library
