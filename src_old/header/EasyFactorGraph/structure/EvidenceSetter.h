/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_EVIDENCE_SETTER_H
#define EFG_STRUCTURE_EVIDENCE_SETTER_H

#include <structure/components/NodesAware.h>
#include <structure/components/EvidenceAware.h>
#include <structure/components/BeliefAware.h>

namespace EFG::strct {
    class EvidencesSetter
        : virtual public NodesAware
        , virtual public EvidenceAware
        , virtual public BeliefAware {
    public:
        /**
         * @brief reset evidence values.
         * The group of observed variables is left unchanged, but the evidence values are updated.
         * @throw when the number of evidences is incosistent
         * @throw when one of the evidence value is incosistent
         */
        void setEvidences(const std::vector<std::size_t>& observations);
    };
}

#endif
