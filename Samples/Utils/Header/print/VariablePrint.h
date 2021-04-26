/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SAMPLE_PRINTER_VARIABLE_H
#define EFG_SAMPLE_PRINTER_VARIABLE_H

#include <categoric/Variable.h>
#include <ostream>

std::ostream& operator<<(std::ostream& s, const EFG::categoric::Variable& group);

#endif
