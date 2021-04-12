/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_XML_IMPORTER_H
#define EFG_NODES_XML_IMPORTER_H

#include <nodes/bases/InsertCapable.h>
#include <nodes/EvidenceChanger.h>

namespace EFG::nodes {
    class XmlImporter
        : virtual public InsertCapable
        , virtual public EvidencesChanger {
    public:
        void importFromXml(const std::string& path, const std::string& file);
    };
}

#endif
