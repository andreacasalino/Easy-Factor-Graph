/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_COMMONS_H
#define EFG_NODES_COMMONS_H

#include <nodes/Node.h>

namespace EFG::nodes {
    void disconnect(Node& node, std::size_t value);

    void reconnnect(Node& node);

    void gatherUnaries(std::set<const distribution::Distribution*>& result, const Node& node);
}

#endif
