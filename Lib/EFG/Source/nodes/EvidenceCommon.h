/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_COMMON_H
#define EFG_NODES_EVIDENCE_COMMON_H

#include <nodes/Node.h>

namespace EFG::nodes {
    void disconnect(Node& node, std::size_t value);

    void reconnnect(Node& node);
}

#endif
