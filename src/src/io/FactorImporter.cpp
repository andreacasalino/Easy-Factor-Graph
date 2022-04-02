/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/io/FactorImporter.h>

#include <XML-Parser/Parser.h>

#include "Utils.h"

namespace EFG::io {

void import_values(distribution::Factor &recipient,
                   const std::string &file_name) {
  auto stream = make_stream(file_name);
  recipient.clear();
  const std::size_t values_size =
      recipient.getVariables().getVariables().size() + 1;
  for_each_line(stream, [&values_size, &recipient](const std::string &line) {
    auto values = xmlPrs::slice_fragments(line);
    if (values.size() != values_size) {
      throw Error{"Invalid file content"};
    }
    const float val = static_cast<float>(std::atof(values.back().c_str()));
    values.pop_back();
    recipient.setImageRaw(parse_combination(values), val);
  });
}
} // namespace EFG::io
