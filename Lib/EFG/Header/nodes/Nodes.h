/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_NODES_H
#define EFG_NODES_NODES_H

#include <distribution/DistributionBase.h>
#include <list>
#include <map>

namespace EFG::nodes {
    class Node {
    public:
        class Connection;
        typedef std::unique_ptr<Connection> ConnectionPtr;

        Node(categoric::VariablePtr variable);

        inline categoric::VariablePtr getVariable() const { return this->variable; };

        inline const std::list<distribution::DistributionPtr>& getUnaryFactors() const { return this->unaryFactors; };
        inline const std::set<ConnectionPtr>& getDisabledConnections() const { return this->disabledConnections; };
        inline const std::set<ConnectionPtr>& getActiveConnections() const { return this->activeConnections; };

        void disconnect(const std::size_t& evidence);
        void reconnect();

        inline const std::size_t* getEvidence() const { return this->evidence.get(); };

    private:
        categoric::VariablePtr variable;
        std::list<distribution::DistributionPtr> unaryFactors;
        std::set<ConnectionPtr> activeConnections;

        std::set<ConnectionPtr> disabledConnections;
        std::unique_ptr<std::size_t> evidence;
    };



    class Node::Connection {
    public:
        Connection& operator=(const Connection&) = delete;
        inline bool operator<(const Connection& o) { return this->neighbour->getVariable()->name() < o.neighbour->getVariable()->name(); };

        static void connect(Node& nodeA, Node& nodeB, distribution::DistributionPtr binaryFactor);

        inline const Node* getNeighbour() const { return this->neighbour; };
        inline Connection* getTwin() const { return this->twin; };

        inline const distribution::DistributionBase* getMessage2This() { return this->message2This.get(); };
        inline void resetMessage(std::unique_ptr<distribution::DistributionBase> message) { this->message2This = std::move(message); };

    private:
        Connection(Node* neighbour, distribution::DistributionPtr factor);

        Node* neighbour;
        distribution::DistributionPtr                   factor;
        //nullptr when the message is not already available
        std::unique_ptr<distribution::DistributionBase> message2This;
        Connection* twin;
    };



    class NodeSet {
    public:
        virtual ~NodeSet() = default;

        categoric::VariablePtr findVariable(const std::string& name) const;

    protected:
        NodeSet() = default;

        std::map<categoric::VariablePtr, Node> nodes;
    };
}

#endif
