/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_CHANGER_H
#define EFG_NODES_EVIDENCE_CHANGER_H

#include <nodes/bases/NodesAware.h>
#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/BeliefAware.h>

namespace EFG::nodes {
    class EvidencesChanger 
        : virtual public NodesAware
        , virtual public EvidenceAware
        , virtual public BeliefAware {
    public:
        /**
         * @brief add a new evidence to the model
         * @param the name of the variable observed
         * @param the value of the evidence
         * @throw when the passed variable name is not found
         * @throw when the passed variable name corresponds to an already set evidence
         * @throw when the passed evidence value is inconsistent
         */
        void addEvidence(const std::string& name, std::size_t value);

        /**
         * @brief reset the evidences, deleting the previous ones.
         * @param the new evidences to assume: <variable name, evidence value>
         * @throw when one of the passed variable name is not found
         * @throw when one of the passed evidence value is inconsistent
         */
        void resetEvidences(const std::map<std::string, std::size_t>& evidences);
    };
}

#endif
