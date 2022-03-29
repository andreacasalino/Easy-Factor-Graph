// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #pragma once

// #include <EasyFactorGraph/distribution/FactorExponential.h>
// #include <EasyFactorGraph/structure/ConnectionsManager.h>

// namespace EFG::strct {
// class FactorsAware : virtual public ConnectionsManager {
// public:
//   const std::unordered_set<distribution::DistributionCnstPtr> &
//   getConstFactors() const {
//     return const_factors;
//   }

// protected:
//   std::unordered_set<distribution::DistributionCnstPtr> const_factors;
// };

// class FactorsAdder : public FactorsAware {
// public:
//   void addConstFactor(const distribution::DistributionCnstPtr &factor);
//   void copyConstFactor(const distribution::Distribution &factor);
// };

// using FactorExponentialPtr =
// std::shared_ptr<distribution::FactorExponential>; using
// FactorExponentialCnstPtr =
//     std::shared_ptr<const distribution::FactorExponential>;

// class WeightTuner;
// struct TunableCluster {
//   std::unique_ptr<WeightTuner> tuner;
//   std::vector<FactorExponentialCnstPtr> factors;
// };

// class FactorsTunableAware : virtual public ConnectionsManager {
// public:
//   const std::vector<TunableCluster> &getTunableFactors() const {
//     return tunable_clusters;
//   }

// protected:
//   std::vector<TunableCluster> tunable_clusters;
// };

// class FactorsAdder : public FactorsAware {
// public:
//   void addConstFactor(const FactorExponentialCnstPtr &factor);
//   void copyConstFactor(const distribution::FactorExponential &factor);
// };

// void absorb();

// void absorb_by_copy();
// } // namespace EFG::strct
