/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/NodeConnection.h>

namespace EFG::nodes {
    Node::Connection::Connection(Node* neighbour, distribution::DistributionPtr factor) 
        : neighbour(neighbour)
        , factor(factor) {
    }

    void Node::Connection::connect(Node& nodeA, Node& nodeB, distribution::DistributionPtr binaryFactor) {
        Connection connA(&nodeB, binaryFactor);
        Connection connB(&nodeA, binaryFactor);
        connA.twin = &connB;
        connB.twin = &connA;
        nodeA.connections.emplace(connA);
        nodeB.connections.emplace(connB);
    }
}
