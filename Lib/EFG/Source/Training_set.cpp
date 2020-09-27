/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <Training_set.h>
#include <XML_Manager.h>
#include <fstream>
#include <cmath>
using namespace std;

namespace EFG {
	

	Training_set::Training_set(const std::string& file_to_import) {

		ifstream f_set(file_to_import);
		if (!f_set.is_open()) {
			f_set.close();
			throw std::runtime_error("invalid training set file");
		}

		string line;
		list<string> slices;

		if (f_set.eof()) {
			f_set.close();
			throw std::runtime_error("invalid training set file"); //found empty training set
		}

		getline(f_set, line);
		XML_reader::splitta_riga(line, &slices);
		vector<string> names;
		names.reserve(slices.size());
		while (!slices.empty()) {
			names.push_back(slices.front());
			slices.pop_front();
		}

		if (f_set.eof()) {
			f_set.close();
			throw std::runtime_error("invalid training set file"); //found empty training set
		}

		std::list<std::pair<size_t* , size_t>> raw;
		size_t k, K;
		while (!f_set.eof()) {
			getline(f_set, line);
			XML_reader::splitta_riga(line, &slices);

			K = slices.size();
			raw.emplace_back(make_pair((size_t*)malloc(sizeof(size_t) * K) , K));
			for(k=0; k<K; ++k){
				raw.back().first[k] = (size_t)atoi(slices.front().c_str());
				slices.pop_front();
			} 
		}

		f_set.close();

		this->__Import(names , raw);

	}

	void Training_set::__Import(const std::vector<std::string>& var_names, const std::list<std::pair<size_t* , size_t>>& combs_raw) {

		if (combs_raw.empty()) throw std::runtime_error("train set empty is invalid");
		if (var_names.empty()) throw std::runtime_error("train set empty is invalid");

		this->var_names = var_names;

		size_t K = this->var_names.size(), k;
		this->max_comb_vals.reserve(K);
		for (k = 0; k < K; ++k) this->max_comb_vals.push_back(0);
		auto it_end = combs_raw.end();
		for (auto it = combs_raw.begin(); it != it_end; ++it) {
			if(it->second != K) throw std::runtime_error("mismatch in the combination sizes");
			this->combs.push_back(it->first);
			for (k = 0; k < K; ++k) {
				if (it->first[k] > this->max_comb_vals[k])
					this->max_comb_vals[k] = it->first[k];
			}
		}
		
	}

	void Training_set::operator+=(const Training_set& tr){

		if(tr.var_names.size() != this->var_names.size()) throw std::runtime_error("cannot copy training set with different combinations sizes");

		size_t k, K = this->var_names.size();
		for(auto it = tr.combs.begin(); it!=tr.combs.end(); ++it){
			this->combs.push_back((size_t*)malloc(K * sizeof(size_t)));
			for (k = 0; k < K; ++k) {
				this->combs.back()[k] = (*it)[k];
				if (this->combs.back()[k] > this->max_comb_vals[k])
					this->max_comb_vals[k] = this->combs.back()[k];
			}
		}

	}

	void Training_set::Print(const std::string& file_name) const {

		ofstream f(file_name);
		if (!f.is_open()) {
			f.close();
			throw std::runtime_error("invalid file to print training set");
		}

		size_t K = this->var_names.size();
		f << this->var_names[0];
		for (size_t k = 1; k < K; ++k) {
			f << " " << this->var_names[k];
		}
		for(auto it = this->combs.begin(); it!=this->combs.end(); ++it){
			f << endl;
			f << (*it)[0];
			for (size_t k = 1; k < K; ++k) f << " " << (*it)[k];
		}

		f.close();

	}

	vector<Categoric_var*> extract(const std::vector<string>& names, const std::vector<size_t>& max_vals, const Graph_Learnable& model_to_train) {
		auto vars = model_to_train.Get_All_variables_in_model();
		if(names.size() != vars.size()) throw std::runtime_error("invalid training set sizes");

		vector<Categoric_var*> result;
		result.reserve(names.size());
		for (size_t k = 0; k < names.size(); ++k) {
			auto temp = model_to_train.Find_Variable(names[k]);
			if(temp == nullptr)  throw std::runtime_error("variable not found in the model");
			result.push_back(temp);
			if(max_vals[k] >= temp->size()) throw std::runtime_error("invalid combinations in training set");
		}
		return result;
	}
	Training_set::Usable_Training_set::Usable_Training_set(Training_set& set, const Graph_Learnable& model_to_train, const float& Sample_percentage) :
	Discrete_Domain(extract(set.var_names , set.max_comb_vals, model_to_train)) , source(&set) , original_size(set.combs.size()) , Observer(&set){

		if(Sample_percentage == 1.f){
			for(auto it=set.combs.begin(); it!=set.combs.end(); ++it) this->domain.push_back(*it);
		}
		else{
			this->original_size = (size_t)ceilf(set.combs.size() * Sample_percentage);
			this->Resample();
		}

	}

	void Training_set::Usable_Training_set::Resample(){

		if(this->original_size >= this->source->combs.size()) return;

		size_t k;
		if(this->domain.size() >= this->original_size){
			for(k = 0; k<this->original_size; ++k) this->domain.pop_front();
		}

		list<size_t*>::iterator sample;
		for(k = 0; k<this->original_size; ++k){
			sample = this->source->combs.begin();
			advance(sample, rand() % this->source->combs.size());
			this->domain.push_front(*sample);
		}

	}

	Training_set::Usable_Training_set::~Usable_Training_set(){

		for(size_t k=0; k<this->original_size; ++k) this->domain.pop_front();

	}

}

