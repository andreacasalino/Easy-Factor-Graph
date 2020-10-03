#include <model/Graph.h>
#include <XML_Manager.h>
#include "../node/NodeFactoryXmlIO.h"
#include <iostream>
using namespace std;

namespace EFG::model {

	Graph::Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file, const node::bp::BeliefPropagator& propagator) :
		NodeFactory(true, propagator) {

		XML_reader* reader = nullptr;
		try { reader = new XML_reader(prefix_config_xml_file + config_xml_file); }
		catch (int) {
			cout << "warninig: file not readable in Graph construction" << endl;
			reader = nullptr;
		}
		if (reader != nullptr) {
			XmlStructureImporter strct(*reader, prefix_config_xml_file);
			this->_Insert(strct.GetStructure());
			this->_SetEvidences(strct.GetObservations());
		}
		delete reader;

	};


}