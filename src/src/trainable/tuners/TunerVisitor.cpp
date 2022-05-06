/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/trainable/tuners/TunerVisitor.h>

namespace EFG::train {
void visit_tuner(
    const TunerPtr &to_visit,
    const std::function<void(const BaseTuner &)> &base_case,
    const std::function<void(const CompositeTuner &)> &composite_case) {
  const auto *as_base = dynamic_cast<const BaseTuner *>(to_visit.get());
  if (nullptr != as_base) {
    base_case(*as_base);
    return;
  }
  const auto *as_composite =
      dynamic_cast<const CompositeTuner *>(to_visit.get());
  if (nullptr != as_composite) {
    composite_case(*as_composite);
    return;
  }
  throw Error{"Unrecognized Tuner"};
}

void visit_tuner(TunerPtr &to_visit,
                 const std::function<void(BaseTuner &)> &base_case,
                 const std::function<void(CompositeTuner &)> &composite_case) {
  auto *as_base = dynamic_cast<BaseTuner *>(to_visit.get());
  if (nullptr != as_base) {
    base_case(*as_base);
    return;
  }
  auto *as_composite = dynamic_cast<CompositeTuner *>(to_visit.get());
  if (nullptr != as_composite) {
    composite_case(*as_composite);
    return;
  }
  throw Error{"Unrecognized Tuner"};
}
} // namespace EFG::train
