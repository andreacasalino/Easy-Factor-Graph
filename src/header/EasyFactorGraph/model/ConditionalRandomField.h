/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/model/RandomField.h>

#ifdef EFG_JSON_IO
#include <nlohmann/json.hpp>
#endif

namespace EFG::model {
#ifdef EFG_XML_IO
struct XmlFileTag {};
static constexpr XmlFileTag XML_FILE_TAG = XmlFileTag{};
#endif

#ifdef EFG_JSON_IO
struct JsonFileTag {};
static constexpr JsonFileTag JSON_FILE_TAG = JsonFileTag{};
#endif

class ConditionalRandomField : protected strct::EvidenceSetter,
                               protected strct::EvidenceRemover,
                               virtual public strct::FactorsAware,
                               protected strct::FactorsAdder,
                               virtual public train::FactorsTunableAware,
                               protected train::FactorsTunableAdder,
                               public strct::GibbsSampler,
                               public strct::QueryManager {
public:
  ConditionalRandomField() = delete;

  ConditionalRandomField(const ConditionalRandomField &o);
  ConditionalRandomField &operator=(const ConditionalRandomField &) = delete;

#ifdef EFG_XML_IO
  /**
   * @brief import the model from an xml file
   * @param the path of the xml to read
   * @throw in case no evidences are set in the file
   */
  ConditionalRandomField(const std::string &file_path, XmlFileTag);
#endif

#ifdef EFG_JSON_IO
  ConditionalRandomField(const std::string &file_path, JsonFileTag);

  ConditionalRandomField(const nlohmann::json &source);
#endif

  ConditionalRandomField(const RandomField &o,
                         const std::unordered_set<std::string> &evidences);

  void setEvidences(const std::vector<std::size_t> &values);

protected:
  std::vector<float> getWeightsGradient_(
      const train::TrainSet::Iterator &train_set_combinations) final;

private:
  const std::vector<std::size_t> evidence_vars_positions;
};
} // namespace EFG::model
