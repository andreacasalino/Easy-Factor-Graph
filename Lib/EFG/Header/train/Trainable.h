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
#include <nodes/bases/StructureTunableAware.h>
#include <list>

namespace EFG::train {
    class TrainHandler {
    public:
        virtual void setTrainSet(const TrainSet& newSet, const std::set<categoric::VariablePtr>& modelVariables) = 0;

        virtual float getGradientAlpha() = 0;
        virtual float getGradientBeta() = 0;

        virtual void setWeight(const float& w) = 0;
    };
    typedef std::unique_ptr<TrainHandler> TrainHandlerPtr;

    class Trainable 
        : virtual public nodes::NodesAware
        , virtual public nodes::StructureTunableAware {
    public:
        void setWeights(const std::vector<float>& w);

        virtual std::vector<float> getGradient(const TrainSet& newSet) = 0;

    protected:
        TrainSet* lastTrainSet = nullptr;

        virtual TrainHandlerPtr makeHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor);
        void insertHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor);

        std::list<TrainHandlerPtr> handlers;
    };
}

#endif
