/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_XML_EXPORTER_H
#define EFG_NODES_XML_EXPORTER_H

#include <nodes/NodesContainer.h>

namespace EFG::nodes {
    class XmlExporter : virtual public NodesContainer {
    public:
        void exportToXml(const std::string& path, const std::string& file);
    };
}

#endif
