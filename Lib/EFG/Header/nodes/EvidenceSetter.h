/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_SETTER_H
#define EFG_NODES_EVIDENCE_SETTER_H

#include <nodes/bases/NodesAware.h>
#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/BeliefAware.h>

namespace EFG::nodes {
    class EvidencesSetter
        : virtual public NodesAware
        , virtual public EvidenceAware
        , virtual public BeliefAware {
    public:
        void setEvidences(const std::vector<std::size_t>& observations);
    };
}

#endif