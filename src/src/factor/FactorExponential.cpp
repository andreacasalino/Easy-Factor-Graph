/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <math.h>

namespace EFG::factor {
FactorExponential::FactorExponential(FunctionPtr data)
    : Immutable{data}, Mutable{data} {}

FactorExponential::FactorExponential(const Factor &factor)
    : FactorExponential(factor, 1.f) {}

namespace {
class ExponentialFunction : public Function {
public:
  ExponentialFunction(const Function &giver, float w)
      : Function{giver.vars()}, weigth{w} {
    std::vector<float> imgs;
    imgs.reserve(info->totCombinations);
    giver.forEachCombination<false>(
        [&imgs](const auto &, float img) { imgs.push_back(img); });
    data_ = std::move(imgs);
  }

  void setWeight(float w) { weigth = w; };
  float getWeight() const { return weigth; };

protected:
  float transform(float input) const override { return expf(weigth * input); }

private:
  float weigth;
};
} // namespace

FactorExponential::FactorExponential(const Factor &factor, float weigth)
    : FactorExponential{
          std::make_shared<ExponentialFunction>(factor.function(), weigth)} {}

void FactorExponential::setWeight(float w) {
  static_cast<ExponentialFunction &>(functionMutable()).setWeight(w);
}

float FactorExponential::getWeight() const {
  return static_cast<const ExponentialFunction &>(function()).getWeight();
}

FactorExponential::FactorExponential(const FactorExponential &o)
    : FactorExponential{
          std::make_shared<ExponentialFunction>(o.function(), o.getWeight())} {}
} // namespace EFG::factor
