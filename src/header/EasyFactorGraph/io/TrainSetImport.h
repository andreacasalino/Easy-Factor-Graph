/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/trainable/TrainSet.h>

#include <string>

namespace EFG::io {
/**
 * @brief Imports the training set from a file.
 * The file should be a matrix of raw values.
 * Each row represent a combination, i.e. a element of the training set to
 * import.
 * @throw in case the passed file is inexistent
 * @throw in case not all the combinations in file have the same size.
 */
train::TrainSet import_train_set(const std::string &file_name);
} // namespace EFG::io
