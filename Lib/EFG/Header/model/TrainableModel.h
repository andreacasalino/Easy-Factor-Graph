///**
// * Author:    Andrea Casalino
// * Created:   01.01.2021
// *
// * report any bug to andrecasa91@gmail.com.
// **/
//
//#ifndef EFG_TRAIN_TRAINABLE_MODEL_H
//#define EFG_TRAIN_TRAINABLE_MODEL_H
//
//#include <nodes/bases/NodesAware.h>
//#include <nodes/BeliefPropagator.h>
//#include <nodes/bases/EvidenceAware.h>
//#include <train/Trainable.h>
//#include <distribution/factor/modifiable/FactorExponential.h>
//
//namespace EFG::model {
//    class TrainableModel
//        : virtual public nodes::NodesAware
//        , public nodes::BeliefPropagator
//        , virtual public nodes::EvidenceAware
//        , public train::Trainable {
//    protected:
//        void insertHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor);
//        void insertHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor, const categoric::Group& potentialSharingWeight);
//    };
//}
//
//#endif