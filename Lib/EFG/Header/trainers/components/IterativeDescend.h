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
#include <chrono>

namespace EFG::train {
    /**
     * @brief Base class for every iterative tuner.
     * The iterations are stopped in case:
        - the l1 norm of the gradient is below a certain threshold
        - the l1 of the difference between the old values of the weights and the new ones is below a certain threshold
     */
    class IterativeDescend
        : public Trainer
        , public virtual ModelAware
        , public virtual IterationsAware
        , public virtual Updatable {
    public:
        void train(Trainable& model, TrainSetPtr trainSet) override;

        /**
         * @brief Set the threshold to consider for the weights improvements 
         * in order to decide whether to arrest the iterations or not.
         * @param the minimum l1 norm acceptable value
         * @throw when the passed value is negative
         */
        void setWeightsTollerance(const float value);
        /**
         * @brief Set the threshold to consider for the gradient
         * in order to decide whether to arrest the iterations or not.
         * @param the minimum l1 norm acceptable value
         * @throw when the passed value is negative
         */
        void setGradientTollerance(const float value);

        /**
         * @brief Display in the console the advancement.
         *  By default the advancement is not displayed
         */
        inline void enablePrintAdvancement() { this->printAdvnc = true; };
        /**
         * @brief Prevent to display in the console the advancement
         *  By default the advancement is not displayed
         */
        inline void disablePrintAdvancement() { this->printAdvnc = false; };

        /**
         * @return the time elapsed in the previous call to train(...)
         */
        inline const std::chrono::milliseconds& getElapsedTime() const { return this->elapsed; };

    protected:
        /**
         * @brief called at every iteration to improve the weights
         */
        virtual void descend() = 0;

        void update() override;
        void reset() override;

    private:
        float weightsTollerance = 0.005f;
        float gradientTollerance = 0.005f;
        bool printAdvnc = false;
        std::chrono::milliseconds elapsed = std::chrono::milliseconds(0);
    };
}

#endif
