/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SAMPLE_PRINTER_DISTRIBUTION_H
#define EFG_SAMPLE_PRINTER_DISTRIBUTION_H

#include <distribution/Distribution.h>
#include <ostream>

std::ostream& operator<<(std::ostream& s, const EFG::distribution::Distribution& distribution);

#endif
