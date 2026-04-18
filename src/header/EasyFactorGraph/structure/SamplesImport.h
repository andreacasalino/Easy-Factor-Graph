/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/Samples.h>

#include <filesystem>
#include <memory>

namespace EFG::structure {
/**
 * @brief Imports the training set from a binary file.
 * Combinations values are encoded as bytes train.
 * Each line is interpreted as a sample.
 * @throw in case the passed file is inexistent
 * @throw in case not all the combinations in file have the same size.
 */
std::unique_ptr<misc::Samples>
load_train_set(const std::filesystem::path &file_name,
               std::size_t samples_per_buffer);

void dump_train_set(const misc::Samples &samples,
                    const std::filesystem::path &file_name);
} // namespace EFG::structure
