/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <string>

class SampleSection {
public:
  SampleSection(const std::string &description,
                const std::string &doc_reference);

  SampleSection(const std::string &description);

  ~SampleSection();
};
