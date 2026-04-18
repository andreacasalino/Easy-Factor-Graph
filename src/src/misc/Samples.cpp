/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/Samples.h>

#include <cstring>

namespace EFG::misc {
Samples::Buffer::Buffer(std::size_t capacity)
    : size{0}, support{new categoric::VarStateSize[capacity]} {}

Samples::Buffer::~Buffer() { delete[] support; }

void Samples::add(std::span<const categoric::VarStateSize> vals) {
  if (vals.size() != sample_len_) {
    throw Error{"Invalid sample length"};
  }
  if (buffers_.empty() || buffers_.rbegin()->size == buffer_capacity_) {
    buffers_.emplace_back(buffer_capacity_);
  }
  auto &recipient = *buffers_.rbegin();
  std::memcpy(recipient.support + recipient.size, vals.data(),
              sample_len_ * sizeof(categoric::VarStateSize));
  recipient.size += sample_len_;
  size_ += 1;
}

Samples::SamplesIter::SamplesIter(const Samples &source)
    : sample_len_{source.sample_len_},
      current_it{source.buffers_.begin()}, end{source.buffers_.end()} {
  updateCurrentView();
}

void Samples::SamplesIter::updateCurrentView() {
  if (current_it != end) {
    current_residual_view = {current_it->support, current_it->size};
  }
}

std::optional<std::span<const categoric::VarStateSize>>
Samples::SamplesIter::next() {
  while (true) {
    if (current_it == end) {
      return std::nullopt;
    }
    if (current_residual_view.empty()) {
      ++current_it;
      updateCurrentView();
      continue;
    }
    std::span<const categoric::VarStateSize> res{current_residual_view.begin(),
                                                 current_residual_view.begin() +
                                                     sample_len_};
    current_residual_view = {current_residual_view.begin() + sample_len_,
                             current_residual_view.end()};
    return res;
  }
}
} // namespace EFG::misc
