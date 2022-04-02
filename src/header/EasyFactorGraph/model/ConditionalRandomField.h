// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #pragma once

// #include <EasyFactorGraph/structure/EvidenceManager.h>
// #include <EasyFactorGraph/structure/FactorsAdder.h>
// #include <EasyFactorGraph/structure/GibbsSampler.h>
// #include <EasyFactorGraph/structure/QueryManager.h>
// #include <EasyFactorGraph/trainable/FactorsTunableManager.h>

// namespace EFG::model {
// class RandomField : protected strct::EvidenceSetter,
//                     protected strct::EvidenceRemover,
//                     virtual public FactorsAware,
//                     public strct::FactorsAdder,
//                     virtual public FactorsTunableAware,
//                     public strct::FactorsTunableAdder,
//                     public strct::GibbsSampler,
//                     public strct::QueryManager {
// public:
//   ConditionalRandomField() = delete;

//   //   /**
//   //    * @throw in case no evidences are present in the passed model
//   //    */
//   //   template <typename Model> explicit ConditionalRandomField(const Model
//   &o)
//   //   {
//   //     this->absorbOther(o);
//   //   };

//   //   ConditionalRandomField(const ConditionalRandomField &o) {
//   //     this->absorbOther(o);
//   //   };
//   ConditionalRandomField &operator=(const ConditionalRandomField &) = delete;

//   //   /**
//   //    * @brief import the model from an xml file
//   //    * @param the path of the xml to read
//   //    * @throw in case no evidences are set in the file
//   //    */
//   //   ConditionalRandomField(const io::FilePath &filePath);

//   void setEvidences(const std::vector<std::size_t> &values);

// private:
//   //   train::TrainHandlerPtr makeHandler(
//   //       std::shared_ptr<distribution::factor::modif::FactorExponential>
//   //       factor) override;

//   //   template <typename Model> void absorbOther(const Model &o) {
//   //     const strct::EvidenceAware *evPtr =
//   //         dynamic_cast<const strct::EvidenceAware *>(&o);
//   //     if (nullptr == evPtr) {
//   //       throw Error("the passed model is not evidence aware");
//   //     }
//   //     const auto &ev = evPtr->getEvidences();
//   //     if (ev.empty()) {
//   //       throw Error("the passed model should have at least 1 evidence");
//   //     }
//   //     this->absorbModel(o, true);
//   //     std::map<std::string, std::size_t> ev2;
//   //     for (auto it = ev.begin(); it != ev.end(); ++it) {
//   //       ev2.emplace(it->first->name(), it->second);
//   //     }
//   //     this->resetEvidences(ev2);
//   //     this->regenerateHandlers();
//   //   };

//   //   // regenerate after knowing the eveidence set
//   //   void regenerateHandlers();
// };
// } // namespace EFG::model
