/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_XML_IMPORTER_H
#define EFG_NODES_XML_IMPORTER_H

#include <nodes/NodesContainer.h>

namespace EFG::nodes {
    class XmlImporter : virtual public NodesContainer {
    public:
        void importFromXml(const std::string& path, const std::string& file);
    };
}

#endif
