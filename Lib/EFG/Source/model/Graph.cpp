#include <model/Graph.h>
#include <Parser.h>
#include "../node/NodeFactoryXmlIO.h"
#include <iostream>
using namespace std;

namespace EFG::model {

	Graph::Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file, const node::bp::BeliefPropagator& propagator) :
		NodeFactory(true, propagator) {

		std::unique_ptr<xmlPrs::Parser> reader;
		try { reader = std::make_unique<xmlPrs::Parser>(prefix_config_xml_file + config_xml_file); }
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