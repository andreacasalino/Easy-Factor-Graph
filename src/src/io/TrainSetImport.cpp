/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/io/TrainSetImport.h>

#include "Utils.h"

namespace EFG::io {

train::TrainSet import_train_set(const std::string &file_name) {
  std::vector<std::vector<std::size_t>> combinations;
  useInStrem(file_name, [&combinations](std::ifstream &stream) {
    std::size_t line_numb = 0;
    std::size_t expected_size = 0;
    for_each_line(stream, [&](const std::string &line) {
      auto &&[combination, img] = parse_combination_image(line);
      if ((expected_size != 0) && (combination.size() != expected_size)) {
        throw Error::make("Invalid combination size at line ",
                          std::to_string(line_numb));
      }
      expected_size = combination.size();
      combinations.emplace_back(std::move(expected_size));
      ++line_numb;
    });
  });
  return train::TrainSet{combinations};
}

} // namespace EFG::io
