#include <model/Graph.h>
#include <Parser.h>
#include "../node/NodeFactoryXmlIO.h"
#include <iostream>
using namespace std;

namespace EFG::model {

	Graph::Graph(const std::string& config_xml_file, const node::bp::BeliefPropagator& propagator) :
		NodeFactory(true, propagator) {
		EFG::node::importInfo readerInfo = EFG::node::createXmlReader(config_xml_file);
		if(nullptr != readerInfo.reader) {
			XmlStructureImporter strct(*readerInfo.reader, readerInfo.prefix);
			this->_Insert(strct.GetStructure(), true);
			this->_SetEvidences(strct.GetObservations());
		}
	};

}