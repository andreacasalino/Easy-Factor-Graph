/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SAMPLE_PRINTER_GROUP_H
#define EFG_SAMPLE_PRINTER_GROUP_H

#include <categoric/Group.h>
#include <ostream>

std::ostream& operator<<(std::ostream& s, const EFG::categoric::Group& group);

#endif
