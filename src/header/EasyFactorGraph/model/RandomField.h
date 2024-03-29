/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsConstManager.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

namespace EFG::model {
/**
 * @brief A complete undurected factor graph storing both constant and tunable
 * factors. Evidences may be changed over the time.
 */
class RandomField : public strct::EvidenceSetter,
                    public strct::EvidenceRemover,
                    public strct::FactorsConstInserter,
                    public train::FactorsTunableInserter,
                    public strct::GibbsSampler,
                    public strct::QueryManager {
public:
  RandomField() = default;

  RandomField(const RandomField &o) { absorb(o, false); };
  RandomField &operator=(const RandomField &) = delete;

  /**
   * @brief Gather all the factors (tunable and constant) of another model and
   * insert/copy them into this object.
   * Tunable factors (Exponential non constant) are recognized and
   * inserted/copied using the train::FactorsTunableAdder interface. All the
   * others inserted/copied using the strct::FactorsAdder interface.
   * @param the model whose factors should be inserted/copied
   * @param when passing true the factors are deep copied, while in the contrary
   * case shallow copies of the smart pointers storing the factors are inserted
   * into this model.
   */
  void absorb(const strct::FactorsAware &to_absorb, bool copy);

protected:
  std::vector<float> getWeightsGradient_(
      const train::TrainSet::Iterator &train_set_combinations) final;
};
} // namespace EFG::model
