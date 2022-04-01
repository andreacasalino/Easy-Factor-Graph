/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::io {
void import_values(distribution::Factor &recipient,
                   const std::string &file_name);
}
