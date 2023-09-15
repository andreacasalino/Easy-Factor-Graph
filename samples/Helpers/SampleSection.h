/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <iostream>
#include <string>

template <typename SampleBody>
void SAMPLE_SECTION(const std::string &DESCRIPTION,
                    const std::string &DOC_SECTION, SampleBody BODY) {
  std::cout << "-------------------------------------------------------------"
               "-------------------------\n\n";
  std::cout << DESCRIPTION;
  if (auto DOC_SECTION_STR = std::string{DOC_SECTION};
      !DOC_SECTION_STR.empty()) {
    std::cout << " , refer to Section " << DOC_SECTION_STR
              << " of the documentation";
  }
  std::cout << "\n-----------------------------------------------------------"
               "---------------------------\n\n";

  BODY();

  std::cout << "\n=========================================================="
               "============================\n";
};
