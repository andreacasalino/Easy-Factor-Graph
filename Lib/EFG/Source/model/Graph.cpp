#include <model/Graph.h>
#include <XML_Manager.h>
#include "../node/NodeFactoryXmlIO.h"
#include <iostream>
using namespace std;

namespace EFG::model {

	Graph::Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file, const node::bp::BeliefPropagator& propagator) :
		NodeFactory(true, propagator) {

		std::unique_ptr<XML_reader> reader;
		try { reader = std::make_unique<XML_reader>(prefix_config_xml_file + config_xml_file); }
		catch (...) {
			cout << "warninig: file not readable in Graph construction" << endl;
			reader.reset();
		}
		if (reader != nullptr) {
			XmlStructureImporter strct(*reader.get(), prefix_config_xml_file);
			this->_Insert(strct.GetStructure(), true);
			this->_SetEvidences(strct.GetObservations());
		}

	};


}