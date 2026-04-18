/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/Structure.h>

namespace EFG::structure {
class ConfigManager {
public:
  /**
   * @param The number of extra workers to spawn when training the model.
   * When passing 0, the available number of cores on this machine is assumed.
   */
  void setWorkersPoolSize(std::size_t w) {
    if (w == 0) {
      // TODO get numb of cores
    }
    context_->config.workers_pool_size = w;
  }

  /**
   * @param The maximum number of iterations for doing model calibration
   */
  void setMaxCalibrationIter(std::size_t iters) {
    context_->config.max_calibration_iter = iters;
  }

  /**
   * @param calibration is stopped whenever the maximum diff of message
   * re-computation is below this threshold
   */
  void setCalibrationTolerance(float toll) {
    context_->config.calibration_tolerance = toll;
  }

  const Config &getconfig() const { return context_->config; }

protected:
  void init(StructurePtr context) { context_ = context; }

private:
  StructurePtr context_;
};
} // namespace EFG::structure
