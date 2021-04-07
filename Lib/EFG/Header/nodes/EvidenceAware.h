/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_AWARE_H
#define EFG_NODES_EVIDENCE_AWARE_H

#include <nodes/NodeSet.h>
#include <list>

namespace EFG::nodes {
    class EvidenceAware : virtual public NodeSet {
    public:
        void addEvidence(categoric::VariablePtr variable, std::size_t value);
        void setEvidences(const std::map<categoric::VariablePtr, const std::size_t>& evidences);

        inline const std::map<categoric::VariablePtr, const std::size_t>& getEvidences() const { return this->evidences; };
        std::set<categoric::VariablePtr> getHiddenVariables() const;

    protected:
        inline const std::list<std::set<Node*>>& getHiddenClusters() const { return this->hiddenClusters; };

    private:
        void recomputeHiddenClusters();

        std::map<categoric::VariablePtr, const std::size_t> evidences;
        std::list<std::set<Node*>> hiddenClusters;
    };
}

#endif
