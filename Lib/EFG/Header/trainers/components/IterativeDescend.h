/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_ITERATIVE_DESCEND_H
#define EFG_TRAINER_ITERATIVE_DESCEND_H

#include <train/Trainer.h>
#include <trainers/components/ModelAware.h>
#include <trainers/components/IterationsAware.h>
#include <trainers/components/Updatable.h>

namespace EFG::train {
    class IterativeDescend
        : public Trainer
        , public virtual ModelAware
        , public virtual IterationsAware
        , public virtual Updatable {
    public:
        void train(Trainable& model, TrainSetPtr trainSet) override;

    protected:
        virtual void descend() = 0;

        void update() override;
        void reset() override;
    };
}

#endif
