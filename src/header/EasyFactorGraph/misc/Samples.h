/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Types.h>

#include <optional>
#include <span>
#include <vector>

namespace EFG::misc {
class Samples {
  struct Buffer {
    Buffer(std::size_t capacity)
        : support{new categoric::VarStateSize[capacity], capacity} {}

    Buffer *add(std::span<const categoric::VarStateSize> vals);

    std::span<categoric::VarStateSize> support;
    std::size_t size{0};
    Buffer *next{nullptr};
  };

public:
  Samples(std::size_t sample_len, std::size_t samples_per_buffer = 1000)
      : sample_len_{sample_len}, buffer_capacity_{sample_len *
                                                  samples_per_buffer} {}

  ~Samples();

  Samples(const Samples &) = delete;
  Samples &operator=(const Samples &) = delete;

  Samples(Samples &&o) noexcept;
  Samples &operator=(Samples &&o) noexcept;

  void add(std::span<const categoric::VarStateSize> vals);

  void add(const std::vector<categoric::VarStateSize> &vals) {
    add(std::span<const categoric::VarStateSize>{vals.begin(), vals.end()});
  }

  struct SamplesIter {
    SamplesIter(const Samples &source);

    std::optional<std::span<const categoric::VarStateSize>> next();

  private:
    void resetResidualView() {
      if (current_) {
        current_residual_view = {current_->support.data(), current_->size};
      } else {
        current_residual_view = {};
      }
    }

    std::size_t sample_len_;
    Buffer *current_;
    std::span<const categoric::VarStateSize> current_residual_view;
  };
  SamplesIter makeIter() const { return {*this}; }

  std::size_t eachSampleSize() const { return sample_len_; }
  std::size_t samplesCount() const { return samples_; }

private:
  void steal(Samples &o);

  void clear();

  std::size_t samples_{0};
  std::size_t sample_len_;
  std::size_t buffer_capacity_;

  Buffer *buffers_head_{nullptr};
  Buffer *buffers_tail_{nullptr};
};
} // namespace EFG::misc
