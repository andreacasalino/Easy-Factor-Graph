/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_NODES_CONTAINER_H
#define EFG_NODES_NODES_CONTAINER_H

#include <nodes/Node.h>

namespace EFG::nodes {
    class NodesContainer {
    public:
        virtual ~NodesContainer() = default;

        NodesContainer(const NodesContainer&) = delete;
        NodesContainer& operator=(const NodesContainer&) = delete;

        categoric::VariablePtr findVariable(const std::string& name) const;

        std::set<categoric::VariablePtr> getVariables() const;
        std::set<categoric::VariablePtr> getHiddenVariables() const;
        inline const std::map<categoric::VariablePtr, const std::size_t>& getEvidences() const { return this->evidences; };

    protected:
        NodesContainer() = default;

        std::map<categoric::VariablePtr, Node> nodes;

        struct HiddenClusters {
            HiddenClusters(const std::set<Node*>& toSplit);
            HiddenClusters() = default;
            void add(const std::list<std::set<Node*>>& toAdd);
            std::list<std::set<Node*>>::iterator find(Node& node);

            std::list<std::set<Node*>> clusters;
        };
        static void add(std::set<Node*>& recipient, const std::set<Node*>& toAdd);

        HiddenClusters hidden;
        std::map<categoric::VariablePtr, const std::size_t> evidences;
    };
}

#endif
