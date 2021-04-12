/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_AWARE_H
#define EFG_NODES_EVIDENCE_AWARE_H

#include <nodes/Node.h>
#include <nodes/bases/Base.h>

namespace EFG::nodes {
    struct HiddenClusters {
        HiddenClusters(const std::set<Node*>& toSplit);
        HiddenClusters() = default;
        void add(const std::list<std::set<Node*>>& toAdd);
        std::list<std::set<Node*>>::iterator find(Node& node);

        std::list<std::set<Node*>> clusters;
    };
    void copyCluster(std::set<Node*>& recipient, const std::set<Node*>& toAdd);

    class EvidenceAware : virtual public Base {
    public:
        std::set<categoric::VariablePtr> getHiddenVariables() const;
        inline const std::map<categoric::VariablePtr, const std::size_t>& getEvidences() const { return this->evidences; };

    protected:
        HiddenClusters hidden;
        std::map<categoric::VariablePtr, const std::size_t> evidences;
    };
}

#endif
