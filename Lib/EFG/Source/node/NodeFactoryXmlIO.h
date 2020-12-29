#include <node/NodeFactory.h>
#include <Parser.h>

namespace EFG::node {

	class Node::NodeFactory::XmlStructureImporter {
	public:
		XmlStructureImporter(xmlPrs::Parser& reader, const std::string& prefix_config_xml_file);
		~XmlStructureImporter() { this->Shapes.clear(); this->ExpShapes.clear(); this->Vars.clear(); };

		inline const Structure& GetStructure() const { return this->ParsedStructure; };
		inline Structure& GetStructure() { return this->ParsedStructure; };
		inline const std::vector<std::pair<std::string, size_t>>& GetObservations() const { return this->Observations; };
	private:
		CategoricVariable*				__FindVar(const std::string& name);
		void						    __ImportShape(const std::string& prefix, xmlPrs::Tag& tag);
	
	// data
		std::list<CategoricVariable>					Vars;
		std::list<pot::Factor>							Shapes;
		std::list<pot::ExpFactor> 						ExpShapes;

		std::vector<std::pair<std::string, size_t>>		Observations;
		Structure										ParsedStructure;
	};

}

