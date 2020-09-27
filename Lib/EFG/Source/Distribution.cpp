#include <Distribution.h>
#include <malloc.h>
#include <fstream>
using namespace std;

namespace EFG {

    I_Discrete::I_Discrete(const std::vector<Categoric_var*>& vars) : Variables(vars){

        if(!Categoric_var::check_all_vars_are_different(vars)) throw std::runtime_error("repeated variable in set");

        size_t K = vars.size();
        this->Variables_observers.reserve(K);
        for(size_t k=0; k<K; ++k) this->Variables_observers.emplace_back(vars[k]);

    }

    void I_Discrete::Substitute_variables(const std::vector<Categoric_var*>& new_var){

        size_t K = new_var.size();
        if(K != this->Variables.size()) throw std::runtime_error("the new domain has not the same caridinality of the previous");
        if(!Categoric_var::check_all_vars_are_different(new_var)) throw std::runtime_error("repeated variable in set");

        for(size_t k=0; k<K; ++k){
            if(new_var[k]->size() != this->Variables[k]->size()) throw std::runtime_error("the new domain does not match for all the variables the sizes");
        }

        this->Variables = new_var;
        this->Variables_observers.clear();
        for(size_t k=0; k<K; ++k) this->Variables_observers.emplace_back(this->Variables[k]);

    }




    Discrete_Distribution::Discrete_Distribution(const std::vector<Categoric_var*>& vars) :
    I_Discrete(vars), Map(comparator(vars.size())) {  }

    Discrete_Distribution::~Discrete_Distribution(){ for(auto it=this->Map.begin(); it!=this->Map.end(); ++it) delete it->second; }

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
	void Import_line(vector<size_t>* comb, float* val, const string& line, const size_t& N_var) {

		list<string> elements = split_string(line);

		if(elements.size() != (N_var + 1)) throw std::runtime_error("invalid line");
		
		*val = (float)atof(elements.back().c_str());
		elements.pop_back();

		comb->clear();
		while (!elements.empty()) {
			comb->push_back((size_t)atoi(elements.front().c_str()));
			elements.pop_front();
		}

	};
    void Discrete_Distribution::import(const std::string& file_to_read){

		ifstream f(file_to_read);
		if (!f.is_open()) {
			f.close();
            throw std::runtime_error("invalid line");
		}

		string line;
		vector<size_t> comb;
		float temp_val;
		bool insert;
        const vector<Categoric_var*>& vars = this->Get_Variables();
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

    bool Discrete_Distribution::comparator::operator()(const Key& a, const Key& b) const{

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

    Discrete_Distribution::Value::~Value() { free(this->combination); };

}
