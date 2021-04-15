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
#include <list>

namespace EFG::train {
    class TrainHandler {
    public:
        virtual void setTrainSet(TrainSetPtr newSet, const std::set<categoric::VariablePtr>& modelVariables) = 0;

        virtual float getGradientAlpha() = 0;
        virtual float getGradientBeta() = 0;
        virtual void setWeight(const float& w) = 0;
    };
    typedef std::unique_ptr<TrainHandler> TrainHandlerPtr;

    class Trainable : virtual public nodes::NodesAware {
    public:
        void setTrainSet(TrainSetPtr newSet);

        void setWeights(const std::vector<float>& w);

        virtual std::vector<float> getGradient() = 0;

        inline TrainSetPtr getTrainSet() const { return this->set; };

    protected:
        TrainSetPtr set;
        std::list<TrainHandlerPtr> handlers;
    };
}

#endif
