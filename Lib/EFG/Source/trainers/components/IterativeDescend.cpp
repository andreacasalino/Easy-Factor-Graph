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
#include <iostream>

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
            if (fabs(data[k]) > res) {
                res = fabs(data[k]);
            }
        }
        return res;
    };

    class TimeCounter {
    public:
        TimeCounter(std::chrono::milliseconds& cumulatedTime)
            : start(std::chrono::high_resolution_clock::now())
            , cumulatedTime(cumulatedTime) {
        };

        ~TimeCounter() {
            this->cumulatedTime += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - this->start);
        };
        
    private:
#ifdef _MSC_VER
        std::chrono::high_resolution_clock::time_point start;
#else
        const std::chrono::time_point<std::chrono::system_clock> start;
#endif
        std::chrono::milliseconds& cumulatedTime;
    };

    void IterativeDescend::train(Trainable& model, TrainSetPtr trainSet) {
        this->elapsed = std::chrono::milliseconds(0);
        this->model = &model;
        this->resetTrainSet(trainSet);
        this->reset();
        std::unique_ptr<Vect> wOld = std::make_unique<Vect>(this->model->getWeights());
        std::unique_ptr<Vect> wAtt;
        for (std::size_t k = 0; k < this->maxIterations; ++k) {
            if (this->printAdvnc) {
                std::cout << "\r iteration:  " << k << " / " << this->maxIterations << std::endl;
            }
            TimeCounter counter(this->elapsed);
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
