/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Group.h>
#include <EasyFactorGraph/misc/Intervals.h>

namespace EFG::factor {
/**
 * @brief Describe a simple correlating domain.
 * All the variables in the passed group should have the same size.
 * The images pertaining to the combinations having all values equal, are
 * assumed equal to 1, all the others to 0.
 * For instance assume to pass a variable set equal to: {<A: size 3>, <B: size
 * 3>, <C: size 3>}. Then, the following combinations map is built:
 * <0,0,0> -> 1
 * <0,0,1> -> 0
 * <0,0,2> -> 0
 *
 * <0,1,0> -> 0
 * <0,1,1> -> 0
 * <0,1,2> -> 0
 *
 * <0,2,0> -> 0
 * <0,2,1> -> 0
 * <0,2,2> -> 0
 *
 * <1,0,0> -> 0
 * <1,0,1> -> 0
 * <1,0,2> -> 0
 *
 * <1,1,0> -> 0
 * <1,1,1> -> 1
 * <1,1,2> -> 0
 *
 * <1,2,0> -> 0
 * <1,2,1> -> 0
 * <1,2,2> -> 0
 *
 * <2,0,0> -> 0
 * <2,0,1> -> 0
 * <2,0,2> -> 0
 *
 * <2,1,0> -> 0
 * <2,1,1> -> 0
 * <2,1,2> -> 0
 *
 * <2,2,0> -> 0
 * <2,2,1> -> 0
 * <2,2,2> -> 1
 */
template <std::size_t N> class SimplyCorrelatedDomainGen {
  static_assert(1 < N,
                "SimplyCorrelatedDomain shall involve at least 2 variables");

public:
  SimplyCorrelatedDomainGen(categoric::VarStateSize var_size)
      : var_size_{var_size} {
    std::fill_n(buffer_.data(), N, 0);
  }

  std::optional<std::pair<categoric::Combination<N>, float>> next() {
    if (buffer_[0] == var_size_) {
      return std::nullopt;
    }
    auto res = std::make_pair(buffer_, 1.f);
    for (auto &val : buffer_) {
      val += 1;
    }
    return res;
  }

private:
  categoric::Combination<N> buffer_;
  std::size_t var_size_;
};

/**
 * @brief Similar to Factor(const categoric::Group &, const
 * UseSimpleCorrelation &), but considering a simple anti-correlation.
 * Therefore, to all combinations having all equal values, an image equal to 0
 * is assigned. All the other ones, are assigned a value equal to 1.
 * For instance assume to pass a variable set equal to: {<A: size 2>, <B: size
 * 2>}. Then, the following combinations map is built:
 * <0,0,0> -> 0
 * <0,0,1> -> 1
 * <0,0,2> -> 1
 *
 * <0,1,0> -> 1
 * <0,1,1> -> 1
 * <0,1,2> -> 1
 *
 * <0,2,0> -> 1
 * <0,2,1> -> 1
 * <0,2,2> -> 1
 *
 * <1,0,0> -> 1
 * <1,0,1> -> 1
 * <1,0,2> -> 1
 *
 * <1,1,0> -> 1
 * <1,1,1> -> 0
 * <1,1,2> -> 1
 *
 * <1,2,0> -> 1
 * <1,2,1> -> 1
 * <1,2,2> -> 1
 *
 * <2,0,0> -> 1
 * <2,0,1> -> 1
 * <2,0,2> -> 1
 *
 * <2,1,0> -> 1
 * <2,1,1> -> 1
 * <2,1,2> -> 1
 *
 * <2,2,0> -> 1
 * <2,2,1> -> 1
 * <2,2,2> -> 0
 */
template <std::size_t N> class SimplyAntiCorrelatedDomainGen {
  static_assert(
      1 < N,
      "SimplyAntiCorrelatedDomainGen shall involve at least 2 variables");

public:
  SimplyAntiCorrelatedDomainGen(categoric::VarStateSize var_size)
      : prev_same_{0} {
    categoric::Group g{categoric::make_same<N>(var_size)};
    domain_len_ = g.combinationsLen();
    // compute the index of the first same values combination that is not zero:
    // <1,1,...,1>
    // Then, positions of same values combinations are equispaced along the
    // domain ...
    delta_ =
        g.combinationIndex(categoric::make_same<N, categoric::VarStateSize>(1));
  }

  std::optional<misc::Chunk> next() {
    if (prev_same_ == (domain_len_ - 1)) {
      return std::nullopt;
    }
    auto next_same = prev_same_ + delta_;
    auto res = std::make_optional(misc::Chunk{prev_same_ + 1, next_same, 1.f});
    prev_same_ = next_same;
    return res;
  }

private:
  std::size_t prev_same_;
  std::size_t delta_;
  std::size_t domain_len_;
};
} // namespace EFG::factor
