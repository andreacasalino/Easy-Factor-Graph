/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_SETTER_H
#define EFG_NODES_EVIDENCE_SETTER_H

#include <nodes/NodesContainer.h>

namespace EFG::nodes {
    class EvidencesSetter : virtual public NodesContainer {
    public:
        void addEvidence(categoric::VariablePtr variable, std::size_t value);
        // previous evidences are deleted
        void setEvidences(const std::map<categoric::VariablePtr, const std::size_t>& evidences);
        void setEvidences(const std::list<std::size_t>& observations);
    };
}

#endif
