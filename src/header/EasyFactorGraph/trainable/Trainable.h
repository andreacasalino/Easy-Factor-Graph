// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef EFG_TRAIN_TRAINABLE_H
// #define EFG_TRAIN_TRAINABLE_H

// #include <categoric/Group.h>
// #include <list>
// #include <structure/components/BeliefAware.h>
// #include <structure/components/NodesAware.h>
// #include <structure/components/StructureTunableAware.h>
// #include <structure/components/ThreadPoolAware.h>
// #include <train/TrainSet.h>

// #include <TrainingTools/Trainer.h>

// namespace EFG::train {
// class TrainHandler {
// public:
//   // recompute alfa part and return it when call getGradientAlpha
//   virtual void
//   setTrainSet(TrainSetPtr trainSet,
//               const std::set<categoric::VariablePtr> &modelVariables) = 0;

//   virtual float getGradientAlpha() = 0;
//   virtual float getGradientBeta() = 0;

//   virtual void setWeight(const float &w) = 0;
// };
// typedef std::unique_ptr<TrainHandler> TrainHandlerPtr;

// /**
//  * @brief An object storing tunable factors, whose weights can be tuned with
//  * training
//  */
// class Trainable : virtual public strct::NodesAware,
//                   virtual public strct::BeliefAware,
//                   virtual public strct::StructureTunableAware
// #ifdef THREAD_POOL_ENABLED
//     ,
//                   virtual public strct::ThreadPoolAware
// #endif
// {
// public:
//   /**
//    * @param the new set of weights to assume for the tunable clusters
//    * @throw when the number of passed weights is inconsistent
//    */
//   void setWeights(const std::vector<float> &w);

//   /**
//    * @param sets equal to 1 the weight of all the tunable clusters
//    */
//   void setOnes();

//   /**
//    * @return the gradient of the weights of the tunable clusters w.r.t. the
//    * passed training set
//    * @throw when the trainSet is inconsistent because have a wrong
//    combinations
//    * size
//    */
//   virtual std::vector<float> getGradient(TrainSetPtr trainSet) = 0;

//   void train(::train::Trainer &solver, TrainSetPtr trainSet);
//   void train(::train::Trainer &solver, TrainSetPtr trainSet,
//              const float percentage);

// protected:
//   virtual TrainHandlerPtr makeHandler(
//       std::shared_ptr<distribution::factor::modif::FactorExponential>
//       factor);
//   void insertHandler(
//       std::shared_ptr<distribution::factor::modif::FactorExponential>
//       factor);

//   std::list<TrainHandlerPtr> handlers;

//   void setTrainSet(TrainSetPtr newSet);
//   class WeightsAware;

// private:
//   TrainSetPtr lastTrainSet;
// };
// } // namespace EFG::train

// #endif
