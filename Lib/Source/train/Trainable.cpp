/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Error.h>
#include <algorithm>
#include <structure/components/StructureAware.h>
#include <train/Trainable.h>
#include <train/handlers/BinaryHandler.h>
#include <train/handlers/CompositeHandler.h>
#include <train/handlers/UnaryHandler.h>

namespace EFG::train {
void Trainable::setTrainSet(TrainSetPtr newSet) {
  if (newSet.get() == this->lastTrainSet.get()) {
    return;
  }
  if (newSet->getSet().front()->size() != this->nodes.size()) {
    throw Error("invalid train set");
  }
  auto vars = this->getVariables();
#ifdef THREAD_POOL_ENABLED
  if (nullptr != this->threadPool) {
    std::for_each(this->handlers.begin(), this->handlers.end(),
                  [&](TrainHandlerPtr &h) {
                    TrainHandlerPtr *hPtr = &h;
                    this->threadPool->push([hPtr, &newSet, &vars]() {
                      (*hPtr)->setTrainSet(newSet, vars);
                    });
                  });
    this->threadPool->wait();
  } else {
#endif
    std::for_each(
        this->handlers.begin(), this->handlers.end(),
        [&newSet, &vars](TrainHandlerPtr &h) { h->setTrainSet(newSet, vars); });
#ifdef THREAD_POOL_ENABLED
  }
#endif
  this->lastTrainSet = newSet;
}

void Trainable::setWeights(const std::vector<float> &w) {
  if (w.size() != this->handlers.size()) {
    throw Error("invalid weigths size");
  }
  this->lastPropagation.reset();
  auto itW = w.begin();
  std::for_each(this->handlers.begin(), this->handlers.end(),
                [&itW](TrainHandlerPtr &h) {
                  h->setWeight(*itW);
                  ++itW;
                });
};

void Trainable::setOnes() {
  this->setWeights(std::vector<float>(this->handlers.size(), 1.f));
}

TrainHandlerPtr Trainable::makeHandler(
    std::shared_ptr<distribution::factor::modif::FactorExponential> factor) {
  const auto &variables = factor->getGroup().getVariables();
  if (1 == variables.size()) {
    return std::make_unique<train::handler::UnaryHandler>(
        this->nodes.find(*variables.begin())->second, factor);
  }
  return std::make_unique<train::handler::BinaryHandler>(
      this->nodes.find(*variables.begin())->second,
      this->nodes.find(*variables.rbegin())->second, factor);
}

void Trainable::insertHandler(
    std::shared_ptr<distribution::factor::modif::FactorExponential> factor) {
  auto newHandler = this->makeHandler(factor);
  auto itF = this->factorsExp.find(factor);
  if (this->handlers.size() == itF->second) {
    // it is a new cluster
    this->handlers.emplace_back(std::move(newHandler));
  } else {
    // should be added into a composite
    auto itHndl = this->handlers.begin();
    std::advance(itHndl, itF->second);
    train::handler::CompositeHandler *compositeIntrf =
        dynamic_cast<train::handler::CompositeHandler *>(itHndl->get());
    if (nullptr == compositeIntrf) {
      *itHndl = std::make_unique<train::handler::CompositeHandler>(
          std::move(*itHndl), std::move(newHandler));
    } else {
      compositeIntrf->addElement(std::move(newHandler));
    }
  }
}

namespace {
::train::Vect convert(const std::vector<float> &buffer) {
  ::train::Vect result(static_cast<Eigen::Index>(buffer.size()));
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    result(static_cast<Eigen::Index>(i)) = buffer[i];
  }
  return result;
}
} // namespace

class Trainable::WeightsAware : public ::train::ParametersAware {
public:
  WeightsAware(train::Trainable &model, TrainSetPtr trainSet) {
    this->model = &model;
    this->trainSet = trainSet;
  };
  WeightsAware(train::Trainable &model, TrainSetPtr trainSet,
               const float percentage)
      : WeightsAware(model, trainSet) {
    this->percentage = std::make_unique<float>(percentage);
  };

  ::train::Vect getParameters() const override {
    return convert(model->getWeights());
  };
  void setParameters(const ::train::Vect &v) override {
    std::vector<float> w;
    w.reserve(v.size());
    for (Eigen::Index i = 0; i < v.size(); ++i) {
      w.push_back(static_cast<float>(v(i)));
    }
    model->setWeights(w);
  };
  ::train::Vect getGradient() const override {
    if (nullptr == percentage) {
      return convert(model->getGradient(trainSet));
    }
    TrainSetPtr sampled =
        std::make_shared<TrainSet>(trainSet->getRandomSubSet(*percentage));
    return convert(model->getGradient(sampled));
  };

private:
  train::Trainable *model;
  TrainSetPtr trainSet;
  std::unique_ptr<float> percentage;
};

void Trainable::train(::train::Trainer &solver, TrainSetPtr trainSet) {
  WeightsAware model(*this, trainSet);
  solver.train(model);
}

void Trainable::train(::train::Trainer &solver, TrainSetPtr trainSet,
                      const float percentage) {
  WeightsAware model(*this, trainSet, percentage);
  solver.train(model);
}
} // namespace EFG::train
