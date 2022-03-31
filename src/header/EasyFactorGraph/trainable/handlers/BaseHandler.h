// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef EFG_TRAIN_HANDLER_BASE_H
// #define EFG_TRAIN_HANDLER_BASE_H

// #include <train/Trainable.h>
// #include <distribution/factor/modifiable/FactorExponential.h>

// namespace EFG::train::handler {
//     class BaseHandler : public TrainHandler {
//     public:
//         void setTrainSet(TrainSetPtr newSet, const
//         std::set<categoric::VariablePtr>& modelVariables) final;

//         inline float getGradientAlpha() final { return this->gradientAlpha;
//         }; inline void setWeight(const float& w) final {
//         this->factor->setWeight(w); };

//     protected:
//         BaseHandler(std::shared_ptr<distribution::factor::modif::FactorExponential>
//         factor);

//         float dotProduct(const std::vector<float>& prob) const;

//         std::shared_ptr<distribution::factor::modif::FactorExponential>
//         factor; float gradientAlpha = 0.f;
//     };
// }

// #endif
