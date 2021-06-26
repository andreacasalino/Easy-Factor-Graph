/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <model/RandomField.h>
#include <algorithm>

namespace EFG::model {
    void RandomField::insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) {
        this->InsertTunableCapable::insertTunable(toInsert);
        this->insertHandler(toInsert);
    }

    void RandomField::insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const std::set<categoric::VariablePtr>& potentialSharingWeight) {
        this->InsertTunableCapable::insertTunable(toInsert, potentialSharingWeight);
        this->insertHandler(toInsert);
    }

    std::vector<float> RandomField::getGradient(train::TrainSetPtr trainSet) {
        this->Trainable::setTrainSet(trainSet);
        if (!this->evidences.empty()) {
            this->resetEvidences({});
        }
        this->lastPropagation.reset();
        std::vector<float> grad;
        grad.resize(this->handlers.size());
        std::size_t pos = 0;
        this->propagateBelief(strct::PropagationKind::Sum);
#ifdef THREAD_POOL_ENABLED
        if (nullptr != this->threadPool) {
            std::for_each(this->handlers.begin(), this->handlers.end(), [this, &grad, &pos](train::TrainHandlerPtr& h) {
                train::TrainHandler* pt = h.get();
                this->threadPool->push([pt, pos, &grad]() { grad[pos] = pt->getGradientAlpha() - pt->getGradientBeta(); });
                ++pos;
            });
            this->threadPool->wait();
        }
        else {
#endif
            std::for_each(this->handlers.begin(), this->handlers.end(), [&grad, &pos](train::TrainHandlerPtr& h) {
                grad[pos] = h->getGradientAlpha() - h->getGradientBeta();
                ++pos;
            });
#ifdef THREAD_POOL_ENABLED
        }
#endif
        return grad;
    }
}
