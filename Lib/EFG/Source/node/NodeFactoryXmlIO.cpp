#include "NodeFactoryXmlIO.h"
#include <algorithm>
using namespace std;

namespace EFG::node {

	struct TunabInfo {
		pot::ExpFactor*				tunab_exp;
		vector<CategoricVariable*>  vars_to_share;
		size_t						cluster_id;
	};
	bool operator==(const TunabInfo& lhs, const TunabInfo& rhs) {

		size_t S = lhs.vars_to_share.size();
		if (S != rhs.vars_to_share.size()) return false;

		if (S == 1) return (lhs.vars_to_share.front() == rhs.vars_to_share.front());
		else {
			if ((rhs.vars_to_share.front() == lhs.vars_to_share.front())
				&& (rhs.vars_to_share.back() == lhs.vars_to_share.back())) return true;
			if ((rhs.vars_to_share.front() == lhs.vars_to_share.back())
				&& (rhs.vars_to_share.back() == lhs.vars_to_share.front())) return true;
			return false;
		}

	}

	void computeTunabClusters(vector<vector<pot::ExpFactor*>>* cluster, list<TunabInfo>& pots) {

		if (pots.empty()) return;

		list<TunabInfo> processed;
		size_t C = 0;
		list<TunabInfo>::const_iterator it, it_end;
		bool found;
		while (!pots.empty()) {
			if (pots.front().vars_to_share.empty()) {
				pots.front().cluster_id = C;
				++C;
			}
			else {
			// find twin in processed
				found = false;
				it_end = processed.end();
				for (it = processed.begin(); it != it_end; ++it) {
					if (*it == pots.front()) {
						pots.front().cluster_id = it->cluster_id;
						found = true;
						break;
					}
				}
				if (!found) {
					found = false;
					it_end = pots.end();
					it = pots.begin();
					++it;
					for (it; it != it_end; ++it) {
						if (*it == pots.front()) {
							found = true;
							break;
						}
					}
					if (!found) throw std::runtime_error("found inexistent potential whose weight is to share");
					pots.front().cluster_id = C;
					++C;
				}
			}
			processed.push_back(pots.front());
			pots.pop_front();
		}

		cluster->reserve(C);
		for (size_t k = 0; k < C; ++k){
			cluster->emplace_back();
			cluster->back().reserve(C);
		}
		it = processed.begin();
		it_end = processed.end();
		for (it; it != it_end; ++it) 
			(*cluster)[it->cluster_id].push_back(it->tunab_exp);

	}
	
    Node::NodeFactory::XmlStructureImporter::XmlStructureImporter(XML_reader& reader, const std::string& prefix_config_xml_file) {

	//import variables
		list<XML_reader::Tag_readable> Nested;
		reader.Get_root().Get_Nested("Variable", &Nested);
        this->Observations.reserve(Nested.size());
		const string* var_flag = NULL;
		std::for_each(Nested.begin(), Nested.end(), [this, &var_flag](XML_reader::Tag_readable& tag) {
			const string* Size = tag.Get_Attribute_first_found("Size");
			const string* Name = tag.Get_Attribute_first_found("name");
			if (this->__FindVar(*Name) != nullptr)  throw std::runtime_error("found multiple variables with the same name");
			this->Vars.emplace_back((size_t)atoi(Size->c_str()), *Name);

			try { var_flag = tag.Get_Attribute_first_found("flag"); }
			catch (int) { var_flag = NULL; }
			if (var_flag != NULL) {
				if (var_flag->compare("O") == 0) {
					this->Observations.emplace_back(make_pair(this->Vars.back().GetName(), 0));
				}
				else if (var_flag->compare("H") != 0) throw std::runtime_error("unrecognized flag value for var tag");
			}
		});

	//import potentials
		list<TunabInfo>	tunab_exp;

		reader.Get_root().Get_Nested("Potential", &Nested);
		get<2>(this->ParsedStructure).reserve(Nested.size());

		const string* w_temp;
		float w_val_temp;
		const string* tun_temp;
		bool is_const;
		list<string> names_to_share;
		for (auto it = Nested.begin(); it != Nested.end(); ++it) {
			this->__ImportShape(prefix_config_xml_file, *it);
			try { w_temp = it->Get_Attribute_first_found("weight"); }
			catch (int) { w_temp = NULL; }

			if(w_temp != NULL) {
				w_val_temp = (float)atof(w_temp->c_str());
				this->ExpShapes.emplace_back(this->Shapes.back() , w_val_temp);
				this->Shapes.pop_back();
				try { tun_temp = it->Get_Attribute_first_found("tunability"); }
				catch (int) { tun_temp = NULL; }
				is_const = false;
				if (tun_temp != NULL) {
					if (tun_temp->compare("Y") == 0) is_const = false;
					else if (tun_temp->compare("N") == 0) is_const = true;
					else throw std::runtime_error("unrecognized parameter");
				}

				if(is_const)
					get<2>(this->ParsedStructure).push_back(&this->ExpShapes.back());
				else {
					tunab_exp.push_back(TunabInfo());
					tunab_exp.back().tunab_exp = &this->ExpShapes.back();
					if (it->Exist_Nested_tag("Share")) {
						it->Get_Nested_first_found("Share").Get_Attributes("var", &names_to_share);
						tunab_exp.back().vars_to_share.reserve(names_to_share.size());
						for (auto itn = names_to_share.begin(); itn != names_to_share.end(); ++itn) {
							tunab_exp.back().vars_to_share.push_back(this->__FindVar(*itn));
							if (tunab_exp.back().vars_to_share.back() == nullptr) throw std::runtime_error("inexistent variable");
						}
						if (this->ExpShapes.back().GetDistribution().GetVariables().size() != tunab_exp.back().vars_to_share.size()) throw std::runtime_error("inconsistent var set");
					}
				}
			}
		}

	// create clusters of tunable potentials
		computeTunabClusters(&get<1>(this->ParsedStructure) , tunab_exp);
		get<0>(this->ParsedStructure).reserve(this->Shapes.size());
		for(auto it=this->Shapes.begin(); it!=this->Shapes.end(); ++it){
			get<0>(this->ParsedStructure).push_back(&(*it));
		}

	}

	CategoricVariable* Node::NodeFactory::XmlStructureImporter::__FindVar(const string& name){

		auto it_end = this->Vars.end();
		for(auto it = this->Vars.begin(); it!=it_end; ++it){
			if(it->GetName().compare(name) == 0) return &(*it);
		}
		return nullptr;

	}

	void Node::NodeFactory::XmlStructureImporter::__ImportShape(const string& prefix, XML_reader::Tag_readable& tag){

		vector<CategoricVariable*> var_involved;
		list<string> names;
		tag.Get_Attributes("var", &names);
		if((names.size() != 1) && (names.size() != 2)) throw std::runtime_error("Only binary or unary potential can be inserted");
		var_involved.reserve(names.size());
		for(auto it=names.begin(); it!= names.end(); ++it){
			var_involved.push_back(this->__FindVar(*it));
			if (var_involved.back() == nullptr) throw std::runtime_error("inexistent variable");
		}

		const string* val = NULL;
		try { val = tag.Get_Attribute_first_found("Source"); }
		catch (int) { val = NULL; }
		if (val != NULL){
			this->Shapes.emplace_back(var_involved, prefix + *val);
			return;
		}  

		try { val = tag.Get_Attribute_first_found("Correlation"); }
		catch (int) { val = NULL; }
		if (val != NULL) {
			if (val->compare("T") == 0) {
				this->Shapes.emplace_back(var_involved, true);
				return;
			}
			else if (val->compare("F") == 0){
				this->Shapes.emplace_back(var_involved, false);
				return;
			}
			else throw std::runtime_error("found potential with invalid options");
		}

		this->Shapes.emplace_back(var_involved);
		list<XML_reader::Tag_readable> distr_vals;
		tag.Get_Nested("Distr_val", &distr_vals);
		list<string> indices_raw;
		vector<size_t> indices;
		const string* temp_D;
		while (!distr_vals.empty()) {
			distr_vals.front().Get_Attributes("v", &indices_raw);
			temp_D = distr_vals.front().Get_Attribute_first_found("D");
			indices.clear();
			indices.reserve(indices_raw.size());
			while (!indices_raw.empty()) {
				indices.push_back((size_t)atoi(indices_raw.front().c_str()));
				indices_raw.pop_front();
			}
			this->Shapes.back().AddValue(indices, (float)atof(temp_D->c_str()));
			distr_vals.pop_front();
		}

	};




	struct DistributionPrinter {
		template<typename P>
		void operator()(XML_reader::Tag_readable& pot_tag, const P& pot) const {
			const distr::DiscreteDistribution& dist = pot.GetDistribution();
			auto vars = dist.GetVariables();
			size_t k, K = vars.size();
			for (k = 0; k < K; ++k) pot_tag.Add_Attribute("var", vars[k]->GetName());
			auto it = dist.getIter();
			itr::forEach<distr::DiscreteDistribution::constIterator>(it, [&pot_tag, &k, &K](distr::DiscreteDistribution::constIterator& itt) {
				auto temp = pot_tag.Add_Nested_and_return_created("Distr_val");
				for (k = 0; k < K; ++k) temp.Add_Attribute("v", to_string(itt->GetIndeces()[k]));
				temp.Add_Attribute("D", to_string(itt->GetValRaw()));
			});
		};
	};
	void Node::NodeFactory::Reprint(const std::string& file_name) const{
		DistributionPrinter printer;

		if (this->Nodes.get_map()->empty())  return;
		
		ofstream f(file_name);
		if (!f.is_open()) {
			f.close();
			throw std::runtime_error("unable to write on the specified file");
		}
		f.close();
		
		XML_reader exporter;
		auto exp_root = exporter.Get_root();

		auto H_vars = this->GetHiddenSet();
		for(auto it=H_vars.begin(); it!=H_vars.end(); ++it){
			auto temp = exp_root.Add_Nested_and_return_created("Variable");
			temp.Add_Attribute("name", (*it)->GetName());
			temp.Add_Attribute("Size", to_string((*it)->size()));
		}

		auto O_vars = this->GetObservationSetVars();
		for(auto it=O_vars.begin(); it!=O_vars.end(); ++it){
			auto temp = exp_root.Add_Nested_and_return_created("Variable");
			temp.Add_Attribute("name", (*it)->GetName());
			temp.Add_Attribute("Size", to_string((*it)->size()));
			temp.Add_Attribute("flag", "O");
		}

		auto structure = this->GetStructure();

		for(auto it = get<0>(structure).begin(); it!=get<0>(structure).end(); ++it){
			auto temp = exp_root.Add_Nested_and_return_created("Potential");
			printer(temp, **it);
		}

		for(auto it = get<2>(structure).begin(); it!=get<2>(structure).end(); ++it){
			auto temp = exp_root.Add_Nested_and_return_created("Potential");
			printer(temp, **it);
			temp.Add_Attribute("weight", to_string((*it)->GetWeight()));
			temp.Add_Attribute("tunability", "N");
		}

		for(auto itc = get<1>(structure).begin(); itc!=get<1>(structure).end(); ++itc){
			for(auto it = itc->begin(); it!=itc->end(); ++it){
				auto temp = exp_root.Add_Nested_and_return_created("Potential");
				printer(temp, **it);
				temp.Add_Attribute("weight", to_string((*it)->GetWeight()));
				if(it!=itc->begin()){
					auto temp2 = temp.Add_Nested_and_return_created("Share");
					auto vars_front = itc->front()->GetDistribution().GetVariables();
					for (auto itV = vars_front.begin(); itV != vars_front.end(); ++itV) {
						temp2.Add_Attribute("var", (*itV)->GetName());
					}
				}
			}
		}

		exporter.Reprint(file_name);

	}

}