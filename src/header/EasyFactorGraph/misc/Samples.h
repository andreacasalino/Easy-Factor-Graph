/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Types.h>

#include <list>
#include <optional>
#include <span>
#include <vector>

namespace EFG::misc {
class Samples {
  struct Buffer {
    Buffer(std::size_t capacity);

    ~Buffer();

    std::size_t size;
    categoric::VarStateSize *support;
  };

  using Buffers = std::list<Buffer>;

public:
  Samples(std::size_t sample_len, std::size_t samples_per_buffer = 1000)
      : sample_len_{sample_len}, buffer_capacity_{sample_len *
                                                  samples_per_buffer} {}

  Samples(const Samples &) = delete;
  Samples &operator=(const Samples &) = delete;

  void add(std::span<const categoric::VarStateSize> vals);

  void add(const std::vector<categoric::VarStateSize> &vals) {
    add(std::span<const categoric::VarStateSize>{vals.begin(), vals.end()});
  }

  struct SamplesIter {
    SamplesIter(const Samples &source);

    std::optional<std::span<const categoric::VarStateSize>> next();

  private:
    void updateCurrentView();

    std::size_t sample_len_;
    Buffers::const_iterator current_it;
    std::span<const categoric::VarStateSize> current_residual_view;
    Buffers::const_iterator end;
  };
  SamplesIter makeIter() const { return {*this}; }

  std::size_t sampleSize() const { return sample_len_; }
  std::size_t size() const { return size_; }

private:
  std::size_t size_{0};
  std::size_t sample_len_;
  std::size_t buffer_capacity_;

  Buffers buffers_;
};
} // namespace EFG::misc
