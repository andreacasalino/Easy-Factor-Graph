/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>
#include <EasyFactorGraph/trainable/tuners/CompositeTuner.h>

namespace EFG::train {
template <typename BasePred, typename CompositePred>
void visitTuner(const Tuner *tuner, BasePred &&base,
                CompositePred &&composite) {
  if (auto *basePtr = dynamic_cast<const BaseTuner *>(tuner); basePtr) {
    base(*basePtr);
  } else {
    composite(static_cast<const CompositeTuner &>(*tuner));
  }
}

template <typename BasePred, typename CompositePred>
void visitTuner(Tuner *tuner, BasePred &&base, CompositePred &&composite) {
  if (auto *basePtr = dynamic_cast<BaseTuner *>(tuner); basePtr) {
    base(*basePtr);
  } else {
    composite(static_cast<CompositeTuner &>(*tuner));
  }
}
} // namespace EFG::train
