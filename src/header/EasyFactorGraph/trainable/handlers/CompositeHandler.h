// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef EFG_TRAIN_HANDLER_COMPOSITE_H
// #define EFG_TRAIN_HANDLER_COMPOSITE_H

// #include <train/Trainable.h>

// namespace EFG::train::handler {
//     class CompositeHandler : public TrainHandler {
//     public:
//         CompositeHandler(TrainHandlerPtr elementA, TrainHandlerPtr elementB);

//         void setTrainSet(TrainSetPtr newSet, const
//         std::set<categoric::VariablePtr>& modelVariables) final;

//         float getGradientAlpha() final;
//         float getGradientBeta() final;
//         void setWeight(const float& w) final;

//         inline void addElement(TrainHandlerPtr element) {
//         this->elements.emplace_back(std::move(element)); };

//     private:
//         std::list<TrainHandlerPtr> elements;
//     };
// }

// #endif
