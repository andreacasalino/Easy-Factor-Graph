/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_EVIDENCE_CHANGER_H
#define EFG_NODES_EVIDENCE_CHANGER_H

#include <nodes/NodesContainer.h>

namespace EFG::nodes {
    class EvidencesChanger : virtual public NodesContainer {
    public:
        void addEvidence(const std::string& name, std::size_t value);
        // previous evidences are deleted
        void resetEvidences(const std::map<std::string, const std::size_t>& evidences);
    };
}

#endif
