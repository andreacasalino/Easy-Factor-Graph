/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/io/ModelComponents.h>
#include <EasyFactorGraph/trainable/TrainSet.h>

#include <filesystem>
#include <fstream>
#include <utility>

namespace EFG::io {
namespace detail {
template <typename T>
void check_stream(const T &stream, const std::filesystem::path &file_name) {
  if (!stream.is_open()) {
    throw Error::make(file_name, " is a non valid file path");
  }
}
} // namespace detail

template <typename Pred>
void useInStrem(const std::filesystem::path &file_name, Pred &&pred) {
  std::ifstream stream{file_name};
  detail::check_stream(stream, file_name);
  pred(stream);
}

template <typename Pred>
void useOutStrem(const std::filesystem::path &file_name, Pred &&pred) {
  std::ofstream stream{file_name};
  detail::check_stream(stream, file_name);
  pred(stream);
}

template <typename Predicate>
void for_each_line(std::istream &stream, const Predicate &pred) {
  std::string line;
  while (!stream.eof()) {
    line.clear();
    std::getline(stream, line);
    pred(line);
  }
}

std::pair<std::vector<std::size_t>, float>
parse_combination_image(const std::string &line);

/**
 * @brief Fill the passed factor with the combinations found in the passed file.
 * The file should be a matrix of raw values.
 * Each row represent a combination and a raw image (as last element) to add to
 * the combination map of the passed distribution.
 * @throw in case the passed file is inexistent
 * @throw in case the parsed combination is inconsitent for the passed
 * distribution
 */
void import_values(factor::Factor &recipient,
                   const std::filesystem::path &file_name);

/**
 * @brief Imports the training set from a file.
 * The file should be a matrix of raw values.
 * Each row represent a combination, i.e. a element of the training set to
 * import.
 * @throw in case the passed file is inexistent
 * @throw in case not all the combinations in file have the same size.
 */
train::TrainSet import_train_set(const std::string &file_name);

struct ImportHelper {
  ImportHelper(Inserters m) : model(m) {}

  Inserters model;

  struct TunableCluster {
    train::FactorExponentialPtr factor;
    categoric::VariablesSet group_owning_w_to_share;
  };
  std::vector<TunableCluster> cumulated;
  void importCumulatedTunable() const;

  void importConst(const factor::ImmutablePtr &factor);

  void importTunable(const train::FactorExponentialPtr &factor,
                     const std::optional<categoric::VariablesSet>
                         &sharing_group = std::nullopt);
};
} // namespace EFG::io
