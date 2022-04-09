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
} // namespace EFG::train
