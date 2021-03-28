#include <distribution/Distribution.h>
#include <fstream>
#include <algorithm>
#include <Error.h>
using namespace std;

namespace EFG::distr {

    DiscreteDistribution::DiscreteDistribution(const std::vector<CategoricVariable*>& vars) :
        Group(vars), Map(comparator(vars.size())) { }

    DiscreteDistribution::DiscreteDistribution(DiscreteDistribution&& o) :
        DiscreteDistribution(o.GetVariables()) { 
        (*this) = std::move(o);
    }

    void DiscreteDistribution::operator=(DiscreteDistribution&& o) {
        const vector<CategoricVariable*>& varThis = this->GetVariables();
        const vector<CategoricVariable*>& varO = o.GetVariables();
        std::size_t S = varThis.size();
        if (varO.size() != S) throw Error("distr::DiscreteDistribution", "move assignment not possible");
        for (std::size_t k = 0; k < S; ++k) {
            if(varThis[k]->size() < varO[k]->size()) throw Error("distr::DiscreteDistribution", "move assignment not possible");
        }
        this->Map = std::move(o.Map);
        
        for (auto it = this->Map.begin(); it != this->Map.end(); ++it) {
            it->second->source = this;
        }

        for (auto it = o.Map.begin(); it != o.Map.end(); ++it) {
            it->second->source = &o;
        }
    }

    DiscreteDistribution::~DiscreteDistribution(){ for(auto it=this->Map.begin(); it!=this->Map.end(); ++it) delete it->second; }

    list<string> split_string(const string& line){

        list<string> elements;
        string temp;
        bool last_was_letter = false;
        for(size_t k=0; k<line.size(); ++k){
            if(line[k] == ' '){
                if(last_was_letter){
                    elements.emplace_back(temp);
                    temp.clear();
                }
                last_was_letter = false;
            }
            else{
                last_was_letter = true;
                temp.push_back(line[k]);
            }
        }
        if(!temp.empty()) elements.emplace_back(temp);
        return elements;

    }
	void Import_line(vector<size_t>* comb, float* val, const string& line, const std::size_t& N_var) {

		list<string> elements = split_string(line);

		if(elements.size() != (N_var + 1)) throw Error("distr::DiscreteDistribution", "invalid line when importing from file");
		
		*val = (float)atof(elements.back().c_str());
		elements.pop_back();

		comb->clear();
		while (!elements.empty()) {
			comb->push_back((size_t)atoi(elements.front().c_str()));
			elements.pop_front();
		}

	};
    void DiscreteDistribution::import(const std::string& file_to_read){

		ifstream f(file_to_read);
		if (!f.is_open()) {
			f.close();
            throw Error("distr::DiscreteDistribution", "invalid file to import");
		}

		string line;
		vector<size_t> comb;
		float temp_val;
		bool insert;
        const vector<CategoricVariable*>& vars = this->GetVariables();
        comb.reserve(vars.size());
		while (!f.eof()) {
			getline(f, line);

			if (line.compare("") != 0) {
				insert = true;
				try {
					Import_line(&comb, &temp_val, line, vars.size());
				}
				catch(...) {
					insert = false;
				}
				if (insert) this->add(comb, temp_val);
			}
		}

		f.close();

    }

    bool DiscreteDistribution::comparator::operator()(const Key& a, const Key& b) const{

        if(a.var_order != nullptr) {
            for(size_t k=0; k<this->N_variables; ++k){
                if(a.combination[(*a.var_order)[k]] < b.combination[k]) return true;
                if(a.combination[(*a.var_order)[k]] > b.combination[k]) return false;
            }
        }
        else if(b.var_order != nullptr){
            for(size_t k=0; k<this->N_variables; ++k){
                if(a.combination[k] < b.combination[(*b.var_order)[k]]) return true;
                if(a.combination[k] > b.combination[(*b.var_order)[k]]) return false;
            }
        }
        else{
            for(size_t k=0; k<this->N_variables; ++k){
                if(a.combination[k] < b.combination[k]) return true;
                if(a.combination[k] > b.combination[k]) return false;
            }
        }
        return false;

    };



    DiscreteDistribution::Value::~Value() { free(this->combination); };



    DiscreteDistribution::Iterator::Iterator(DiscreteDistribution& toIterate)
        : EFG::itr::TypedIterator<Distribution_map::iterator>(toIterate.Map.begin() , toIterate.Map.end()) {
    }

    DiscreteDistribution::constIterator::constIterator(const DiscreteDistribution& toIterate)
        : EFG::itr::TypedIterator<Distribution_map::const_iterator>(toIterate.Map.begin(), toIterate.Map.end()) {
    }

}
