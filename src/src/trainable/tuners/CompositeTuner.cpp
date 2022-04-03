/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/trainable/tuners/CompositeTuner.h>

#include <algorithm>

namespace EFG::train {
CompositeTuner::CompositeTuner(TunerPtr elementA, TunerPtr elementB) {
  addElement(std::move(elementA));
  addElement(std::move(elementB));
}

std::vector<FactorExponentialPtr> CompositeTuner::getFactors() const {
  std::vector<FactorExponentialPtr> result;
  for (const auto &tuner : elements) {
    for (const auto &factor : tuner->getFactors()) {
      result.push_back(factor);
    }
  }
  return result;
}

void CompositeTuner::addElement(TunerPtr element) {
  if (nullptr == element) {
    throw Error{"null tuner"};
  }
  elements.emplace_back(std::move(element));
}

void CompositeTuner::setTrainSetIterator(const TrainSet::Iterator &iter) {
  for (auto &element : elements) {
    element->setTrainSetIterator(iter);
  }
}

float CompositeTuner::getGradientAlpha() {
  float grad = 0.f;
  for (auto &element : elements) {
    grad += element->getGradientAlpha();
  }
  return grad;
}

float CompositeTuner::getGradientBeta() {
  float grad = 0.f;
  for (auto &element : elements) {
    grad += element->getGradientBeta();
  }
  return grad;
}

void CompositeTuner::setWeight(const float &w) {
  for (auto &element : elements) {
    element->setWeight(w);
  }
}

bool CompositeTuner::isHereGroup(const categoric::VariablesSet &group) const {
  return std::find_if(elements.begin(), elements.end(),
                      [&group](const TunerPtr &element) {
                        return element->isHereGroup(group);
                      }) != elements.end();
}
} // namespace EFG::train
