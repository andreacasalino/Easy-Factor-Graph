/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/trainable/TrainSet.h>

namespace EFG::io {
train::TrainSet import_train_set(const std::string &file_name);
}
