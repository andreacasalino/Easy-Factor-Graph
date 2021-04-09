/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/Node.h>

namespace EFG::nodes {
    Connection::Connection(distribution::DistributionPtr factor, std::unique_ptr<distribution::Distribution> message)
        : factor(factor)
        , message2This(std::move(message)) {
    };

    Connection::Connection(Connection&& o) {
        this->factor = o.factor;
        this->message2This = std::move(o.message2This);
    };

    Node::Node(categoric::VariablePtr var) { 
        variable = var; 
    };
}
