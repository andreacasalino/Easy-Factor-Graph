/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <trainers/components/IterativeDescend.h>
#include <trainers/Commons.h>
#include <Error.h>
#include <math.h>

namespace EFG::train {
    void IterativeDescend::setWeightsTollerance(const float value) {
        if (value < 0.f) {
            throw Error("Negative tollerance value");
        }
        this->weightsTollerance = value;
    };

    void IterativeDescend::setGradientTollerance(const float value) {
        if (value < 0.f) {
            throw Error("Negative tollerance value");
        }
        this->gradientTollerance = value;
    };

    float l1Norm(const Vect& v) {
        float res = 0.f;
        const float* data = v.data();
        for (std::size_t k = 0; k < v.size(); ++k) {
            if (fabsf(data[k]) > res) {
                res = fabsf(data[k]);
            }
        }
        return res;
    };

    void IterativeDescend::train(Trainable& model, TrainSetPtr trainSet) {
        this->model = &model;
        this->resetTrainSet(trainSet);
        this->reset();
        std::unique_ptr<Vect> wOld = std::make_unique<Vect>(this->model->getWeights());
        std::unique_ptr<Vect> wAtt;
        for (std::size_t k = 0; k < this->maxIterations; ++k) {
            this->descend();
            this->update();
            if (l1Norm(this->lastGrad) < this->gradientTollerance) {
                break;
            }
            wAtt = std::make_unique<Vect>(this->model->getWeights());
            if (l1Norm(*wOld - *wAtt) < this->weightsTollerance) {
                break;
            }
            std::swap(wOld, wAtt);
        }
        this->model = nullptr;
    }

    void IterativeDescend::update() {
        ++this->doneIterations;
#ifdef ADVANCED_TRAINERS_ENABLED
        this->lastWeights = this->model->getWeights();
        this->lastGrad = this->getGradient();
#endif
    };

    void IterativeDescend::reset() {
        this->doneIterations = 1;
#ifdef ADVANCED_TRAINERS_ENABLED
        this->lastWeights = this->model->getWeights();
        this->lastGrad = this->getGradient();
#endif
    };
}
