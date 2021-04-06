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
    struct Evidence {
        bool operator<(const Evidence& o) const;

        categoric::VariablePtr variable;
        std::size_t value;
    };
    

    class EvidenceAware : virtual public NodesBase {
    public:
        // the same variables inside the model shoudld be used
        void setEvidences(const std::set<Evidence>& evidences);
        void setEvidences(const std::set<categoric::VariablePtr>& variables, const std::list<std::size_t>& values);
        void setEvidences(const std::list<std::size_t>& values);

        inline const std::set<Evidence>& getEvidences() const { return this->evidences; };
        std::set<categoric::VariablePtr> getHiddenVariables() const;

    protected:
        inline const std::list<std::set<Node>>& getHiddenVariablesClusters() const { return this->hiddenClusters; };

    private:
        std::set<Evidence> evidences;
        std::list<std::set<Node>> hiddenClusters;
    };
}

#endif
