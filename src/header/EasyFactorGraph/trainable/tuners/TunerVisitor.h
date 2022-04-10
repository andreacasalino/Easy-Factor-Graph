/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/trainable/tuners/BinaryTuner.h>
#include <EasyFactorGraph/trainable/tuners/CompositeTuner.h>

#include <functional>

namespace EFG::train {
void visit_tuner(
    const TunerPtr &to_visit,
    const std::function<void(const BaseTuner &)> &base_case,
    const std::function<void(const CompositeTuner &)> &composite_case);

void visit_tuner(TunerPtr &to_visit,
                 const std::function<void(BaseTuner &)> &base_case,
                 const std::function<void(CompositeTuner &)> &composite_case);
} // namespace EFG::train
