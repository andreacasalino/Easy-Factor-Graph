/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/io/TrainSetImporter.h>
#include <EasyFactorGraph/io/Utils.h>

#include <XML-Parser/Parser.h>

namespace EFG::io {
train::TrainSet import_train_set(const std::string &file_name) {
  auto stream = make_in_stream(file_name);
  std::vector<categoric::Combination> combinations;
  std::size_t line_numb = 0;
  for_each_line(stream, [&combinations, &line_numb](const std::string &line) {
    auto comb = parse_combination(xmlPrs::slice_fragments(line));
    if ((!combinations.empty()) && (combinations.front().data().size())) {
      throw Error{"Invalid combination size at line ",
                  std::to_string(line_numb)};
    }
    combinations.emplace_back(std::move(comb));
    ++line_numb;
  });
  return train::TrainSet{combinations};
}
} // namespace EFG::io
