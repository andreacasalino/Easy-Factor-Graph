/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_TRAINABLE_H
#define EFG_TRAIN_TRAINABLE_H

#include <categoric/Group.h>
#include <train/TrainSet.h>
#include <nodes/bases/NodesAware.h>
#include <nodes/bases/ThreadPoolAware.h>
#include <nodes/bases/StructureTunableAware.h>
#include <list>

namespace EFG::train {
    class TrainHandler {
    public:
        // recompute alfa part and return it when call getGradientAlpha
        virtual void setTrainSet(TrainSetPtr trainSet, const std::set<categoric::VariablePtr>& modelVariables) = 0;

        virtual float getGradientAlpha() = 0;
        virtual float getGradientBeta() = 0;

        virtual void setWeight(const float& w) = 0;
    };
    typedef std::unique_ptr<TrainHandler> TrainHandlerPtr;

    class Trainable 
        : virtual public nodes::NodesAware
        , virtual public nodes::StructureTunableAware
        , virtual public nodes::ThreadPoolAware {
    public:
        void setWeights(const std::vector<float>& w);
        void setOnes();

        virtual std::vector<float> getGradient(TrainSetPtr trainSet) = 0;

    protected:
        virtual TrainHandlerPtr makeHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor);
        void insertHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor);

        std::list<TrainHandlerPtr> handlers;

        void setTrainSet(TrainSetPtr newSet);
        inline TrainSetPtr getLastTrainSet() const { return this->lastTrainSet; }

    private:
        TrainSetPtr lastTrainSet;
    };
}

#endif
