/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_AWARE_H
#define EFG_NODES_EVIDENCE_AWARE_H

#include <nodes/NodesBase.h>
#include <list>

namespace EFG::nodes {
    class EvidenceAware : virtual public NodesBase {
    public:
        categoric::Group getEvidenceVariables() const;
        // same order inside Group is assumed
        std::list<std::size_t> getEvidenceValues() const;
        std::list<std::pair<std::size_t, categoric::VariablePtr>> getEvidence() const;

        void setEvidenceVariables(const categoric::Group& vars);
        // same order inside last set Group is assumed
        void setEvidenceValues(const std::list<std::size_t>& vals);
        void setEvidence(const std::list<std::pair<std::size_t, categoric::VariablePtr>>& evidences);

    protected:
        inline const std::list<std::set<Node>>& getHiddenVariablesClusters() const { return this->hiddenVariablesClusters; };

    private:
        std::list<std::set<Node>> hiddenVariablesClusters;
    };
}

#endif
