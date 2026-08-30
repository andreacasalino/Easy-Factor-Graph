/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/Samples.h>

#include <cstring>

namespace EFG::misc {
Samples::Buffer *
Samples::Buffer::add(std::span<const categoric::VarStateSize> vals) {
  std::size_t residual = support.size() - size;
  if (vals.size() <= residual) {
    std::memcpy(support.data() + size, vals.data(),
                vals.size() * sizeof(categoric::VarStateSize));
    size += vals.size();
    return this;
  } else {
    auto *res = new Buffer{support.size()};
    next = res;
    res->add(vals);
    return res;
  }
}

void Samples::clear() {
  auto *current = buffers_head_;
  while (current) {
    delete[] current->support.data();
    auto *next = current->next;
    delete current;
    current = next;
  }
}

Samples::~Samples() { clear(); }

void Samples::steal(Samples &o) {
  buffers_head_ = o.buffers_head_;
  buffers_tail_ = o.buffers_tail_;
  samples_ = o.samples_;
  o.samples_ = 0;
  o.buffers_head_ = nullptr;
  o.buffers_tail_ = nullptr;
}

Samples::Samples(Samples &&o) noexcept
    : Samples{o.sample_len_, o.buffer_capacity_ / o.sample_len_} {
  steal(o);
}

Samples &Samples::operator=(Samples &&o) noexcept {
  clear();
  steal(o);
  return *this;
}

void Samples::add(std::span<const categoric::VarStateSize> vals) {
  if (vals.size() != sample_len_) {
    throw Error{"Invalid sample length"};
  }

  if (!buffers_head_) {
    buffers_head_ = new Buffer{buffer_capacity_};
    buffers_tail_ = buffers_head_;
  }

  buffers_tail_ = buffers_tail_->add(vals);
}

Samples::SamplesIter::SamplesIter(const Samples &source)
    : sample_len_{source.sample_len_}, current_{source.buffers_head_} {
  resetResidualView();
}

std::optional<std::span<const categoric::VarStateSize>>
Samples::SamplesIter::next() {
  while (current_) {
    if (current_residual_view.empty()) {
      current_ = current_->next;
      resetResidualView();
      continue;
    }

    std::span<const categoric::VarStateSize> res{current_residual_view.begin(),
                                                 current_residual_view.begin() +
                                                     sample_len_};
    current_residual_view = {current_residual_view.begin() + sample_len_,
                             current_residual_view.end()};
    return res;
  }
  return std::nullopt;
}
} // namespace EFG::misc
