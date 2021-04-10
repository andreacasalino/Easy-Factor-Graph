/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SAMPLE_PRINTER_PROBABILITY_DISTRIBUTION_H
#define EFG_SAMPLE_PRINTER_PROBABILITY_DISTRIBUTION_H

#include <vector>
#include <ostream>

std::ostream& operator<<(std::ostream& s, const std::vector<float>& distribution);

#endif
