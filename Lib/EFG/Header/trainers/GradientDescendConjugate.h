/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#ifndef EFG_TRAINER_CONJUGATE_GRADIENT_H
#define EFG_TRAINER_CONJUGATE_GRADIENT_H

#include <trainers/components/IterativeDescend.h>
#include <trainers/strategies/BasicTrainSet.h>
#include <trainers/strategies/YundaSearcher.h>

namespace EFG::train {
    class GradientDescendConjugate
        : public IterativeDescend {
    protected:
        void reset() override;

        void descend() override;

        virtual float computeBeta() = 0;

        Vect lastDirection;
    };



    template<typename TrainSetT = BasicTrainSet, typename LineSearcherT = YundaSearcher>
    class FletcherBroyden
        : public GradientDescendConjugate
        , public TrainSetT
        , public LineSearcherT {
        static_assert(std::is_base_of<TrainSetT, BasicTrainSet>::value, "TrainSetT should be a form of BasicTrainSet");
        static_assert(std::is_base_of<LineSearcherT, LineSearcher>::value, "LineSearcherT should be a form of LineSearcher");
    protected:
        inline float computeBeta() override { return };
    };

    template<typename TrainSetT = BasicTrainSet, typename LineSearcherT = YundaSearcher>
    class PolakRibiere
        : public GradientDescendConjugate
        , public TrainSetT
        , public LineSearcherT {
        static_assert(std::is_base_of<TrainSetT, BasicTrainSet>::value, "TrainSetT should be a form of BasicTrainSet");
        static_assert(std::is_base_of<LineSearcherT, LineSearcher>::value, "LineSearcherT should be a form of LineSearcher");
    protected:
        inline float computeBeta() override { return };
    };
}

#endif
#endif