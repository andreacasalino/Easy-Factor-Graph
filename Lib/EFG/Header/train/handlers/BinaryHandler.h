/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_HANDLER_BINARY_H
#define EFG_TRAIN_HANDLER_BINARY_H

#include <train/handlers/BaseHandler.h>
#include <nodes/Node.h>

namespace EFG::train::handler {
    class BinaryHandler : public BaseHandler {
    public:
        BinaryHandler(nodes::Node& nodeA, nodes::Node& nodeB, std::shared_ptr<distribution::factor::modif::FactorExponential> factor);

        float getGradientBeta() final;

    protected:
        nodes::Node* nodeA;
        nodes::Node* nodeB;
    };
}

#endif
