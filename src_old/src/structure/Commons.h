/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_COMMONS_H
#define EFG_STRUCTURE_COMMONS_H

#include <structure/Node.h>

namespace EFG::strct {
    void disconnect(Node& node, std::size_t value);

    void reconnnect(Node& node);

    void gatherUnaries(std::set<const distribution::Distribution*>& result, const Node& node);

    void resetMessages(Node& node);
}

#endif
