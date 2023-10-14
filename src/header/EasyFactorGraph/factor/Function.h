/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/misc/Visitor.h>

#include <unordered_map>
#include <variant>

namespace EFG::factor {
class Function {
public:
  Function(const categoric::Group &variables);

  virtual ~Function() = default;

  void cloneImages(const Function &o) { data_ = o.data_; }

  void set(const std::vector<std::size_t> &combination, float image);

  float findTransformed(const std::vector<std::size_t> &combination) const;

  float findImage(const std::vector<std::size_t> &combination) const;

  void clear() { data_ = makeSparseContainer(); }

  // Pred(const std::vector<std::size_t>&, float)
  template <bool UseTransformed, typename Pred>
  void forEachCombination(Pred &&pred) const {
    categoric::GroupRange range{info->sizes};
    VisitorConst<SparseContainer, DenseContainer>{
        [&](const SparseContainer &c) {
          categoric::for_each_combination(
              range, [&](const std::vector<std::size_t> &comb) {
                auto it = c.find(comb);
                float img = it == c.end() ? 0 : it->second;
                if constexpr (UseTransformed) {
                  img = this->transform(img);
                }
                pred(comb, img);
              });
        },
        [&](const DenseContainer &c) {
          auto cIt = c.begin();
          categoric::for_each_combination(
              range, [&](const std::vector<std::size_t> &comb) {
                float img = *cIt;
                if constexpr (UseTransformed) {
                  img = this->transform(img);
                }
                pred(comb, img);
                ++cIt;
              });
        }}
        .visit(data_);
  }

  // Pred(const std::vector<std::size_t>&, float)
  template <bool UseTransformed, typename Pred>
  void forEachNonNullCombination(Pred &&pred) const {
    VisitorConst<SparseContainer, DenseContainer>{
        [&](const SparseContainer &c) {
          for (const auto &[comb, img] : c) {
            float img2 = img;
            if constexpr (UseTransformed) {
              img2 = this->transform(img2);
            }
            pred(comb, img2);
          }
        },
        [&](const DenseContainer &c) {
          categoric::GroupRange range{info->sizes};
          auto cIt = c.begin();
          categoric::for_each_combination(
              range, [&](const std::vector<std::size_t> &comb) {
                if (*cIt != 0) {
                  float img = *cIt;
                  if constexpr (UseTransformed) {
                    img = this->transform(img);
                  }
                  pred(comb, img);
                }
                ++cIt;
              });
        }}
        .visit(data_);
  }

  struct Info {
    std::vector<std::size_t> sizes;
    std::size_t totCombinations;

    // something to dynamically pass from a sparse to a dense distribution when
    // the number of combinations increase enough
    std::size_t critical_size;
  };
  const Info &getInfo() const { return *info; }

  const auto &vars() const { return variables_; }
  auto &vars() { return variables_; }

  struct CombinationHasher {
    std::shared_ptr<const Info> info;

    std::size_t operator()(const std::vector<std::size_t> &comb) const;
  };

protected:
  categoric::Group variables_;
  std::shared_ptr<const Info> info;

  /**
   * @brief applies a specific function to obtain the image from the passed rwa
   * value
   * @param the raw value to convert
   * @return the converted image
   */
  virtual float transform(float input) const { return input; }

  using SparseContainer =
      std::unordered_map<std::vector<std::size_t>, float, CombinationHasher>;

  SparseContainer makeSparseContainer();

  using DenseContainer = std::vector<float>;

  std::variant<SparseContainer, DenseContainer> data_;
};

std::shared_ptr<const Function::Info> make_info(const categoric::Group &vars);

using FunctionPtr = std::shared_ptr<Function>;
} // namespace EFG::factor
