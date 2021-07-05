/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <trainers/components/IterativeDescend.h>
#include <trainers/Commons.h>
#include <Error.h>

namespace EFG::train {
    constexpr float MIN_GRAD_L1 = 0.001f;

    float l1Norm(const Vect& v) {
        float res = 0.f;
        const float* data = v.data();
        for (std::size_t k = 0; k < v.size(); ++k) {
            if (std::fabsf(data[k]) > res) {
                res = std::fabsf(data[k]);
            }
        }
        return res;
    };

    void IterativeDescend::train(Trainable& model, TrainSetPtr trainSet) {
        this->model = &model;
        this->resetTrainSet(trainSet);
        this->reset();
        for (std::size_t k = 0; k < this->maxIterations; ++k) {
            this->descend();
            this->update();
            if (l1Norm(this->lastGrad) < MIN_GRAD_L1) {
                break;
            }
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
