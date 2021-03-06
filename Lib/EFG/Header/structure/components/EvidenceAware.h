/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_EVIDENCE_AWARE_H
#define EFG_STRUCTURE_EVIDENCE_AWARE_H

#include <structure/Node.h>
#include <Component.h>

namespace EFG::strct {
    struct HiddenClusters {
        explicit HiddenClusters(const std::set<Node*>& toSplit);
        HiddenClusters() = default;
        void add(const std::list<std::set<Node*>>& toAdd);
        std::list<std::set<Node*>>::iterator find(Node& node);

        std::list<std::set<Node*>> clusters;
    };
    void copyCluster(std::set<Node*>& recipient, const std::set<Node*>& toAdd);

    class EvidenceAware : virtual public Component {
    public:
        std::set<categoric::VariablePtr> getHiddenVariables() const;
        std::set<categoric::VariablePtr> getObservedVariables() const;
        inline const std::map<categoric::VariablePtr, std::size_t>& getEvidences() const { return this->evidences; };

    protected:
        /**
         * @brief Clusters of hidden node. Each cluster is a group of connected hidden nodes.
         * Nodes in different clusters are not currently connected (due to the model structure or the kind of evidences currently set)
         */
        HiddenClusters hidden;
        std::map<categoric::VariablePtr, std::size_t> evidences;
    };
}

#endif
