/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::io {
/**
 * @brief Fill the passed factor with the combinations found in the passed file.
 * The file should be a matrix of raw values.
 * Each row represent a combination and a raw image (as last element) to add to
 * the combination map of the passed distribution.
 * @throw in case the passed file is inexistent
 * @throw in case the parsed combination is inconsitent for the passed
 * distribution
 */
void import_values(distribution::Factor &recipient,
                   const std::string &file_name);
} // namespace EFG::io
