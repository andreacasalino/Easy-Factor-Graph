/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_CONNECTION_H
#define EFG_NODES_CONNECTION_H

#include <nodes/Node.h>

namespace EFG::nodes {
    class Node::Connection {
    public:
        Connection& operator=(const Connection&) = delete;

        static void connect(Node& nodeA, Node& nodeB, distribution::DistributionPtr binaryFactor);

        inline const Node* getNeighbour() const { return this->neighbour; };
        inline Connection* getTwin() const { return this->twin; };

        inline const distribution::DistributionBase* getMessage2This() { return this->message2This.get(); };
        inline void setMessage2This(std::unique_ptr<distribution::DistributionBase> message) { this->message2This = std::move(message); };

    private:
        Connection(Node* neighbour, distribution::DistributionPtr factor);
        Connection(const Connection&) = default;

		Node*											neighbour;
        distribution::DistributionPtr                   factor;
        //nullptr when the message is not already available
        std::unique_ptr<distribution::DistributionBase> message2This;
		Connection*							            twin;
    };
}

#endif
