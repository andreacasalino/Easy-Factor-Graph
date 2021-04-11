/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_XML_EXPORTER_H
#define EFG_NODES_XML_EXPORTER_H

#include <nodes/bases/NodesAware.h>
#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/StructureAware.h>

namespace EFG::nodes {
    class XmlExporter 
        : virtual public EvidenceAware
        , virtual public StructureAware {
    public:
        void exportToXml(const std::string& path, const std::string& name = "") const;
    };
}

#endif
