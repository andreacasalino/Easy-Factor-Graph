/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <SampleSection.h>

#include <iostream>

SampleSection::SampleSection(const std::string &description,
                             const std::string &doc_reference) {
  std::cout
      << "\n\n--------------------------------------------------------\n\n";
  std::cout << description;
  if (!doc_reference.empty()) {
    std::cout << " , refer to the Section " << doc_reference
              << " of the documentation";
  }
  std::cout << "\n--------------------------------------------------------\n\n";
}

SampleSection::~SampleSection() {
  std::cout << "\n--------------------------------------------------------\n\n";
}
