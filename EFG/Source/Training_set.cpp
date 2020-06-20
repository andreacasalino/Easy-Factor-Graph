/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Training_set.h"
#include "../XML_Manager/XML_Manager.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
using namespace std;

namespace EFG {

	template<typename T>
	void check_variables(const list<T>& vars) {

		if (vars.empty()) throw 0;

		//check all variables are different
		auto it_end = vars.end();
		auto it2 = vars.begin();
		for (auto it = vars.begin(); it != it_end; it++) {
			for (it2 = vars.begin(); it2 != it; it2++) {
				if (*it2 == *it) throw 1;
			}
		}

	}

	I_Potential::combinations::_Training_set_t::_Training_set_t(const std::string& file_to_import) {

		ifstream f_set(file_to_import);
		if (!f_set.is_open()) {
			f_set.close();
			throw 0; //impossible to open training set file
		}

		string line;
		list<string> slices;

		if (f_set.eof()) {
			f_set.close();
			throw 1; //found empty training set
		}

		getline(f_set, line);
		XML_reader::splitta_riga(line, &this->Variable_names);

		if (f_set.eof()) {
			f_set.close();
			throw 1; //found empty training set
		}

		list<list<size_t>> vals;
		while (!f_set.eof()) {
			getline(f_set, line);
			slices.clear();
			XML_reader::splitta_riga(line, &slices);

			vals.push_back(list<size_t>());
			while (!slices.empty()) {
				vals.back().push_back((size_t)atoi(slices.front().c_str()));
				slices.pop_front();
			}
		}

		f_set.close();

		this->__Import(vals);

	}

	I_Potential::combinations::_Training_set_t::_Training_set_t(const std::list<std::list<size_t>>& samples, const std::list<std::string>& var_names) {

		this->Variable_names = var_names;
		this->__Import(samples);

	}

	void I_Potential::combinations::_Training_set_t::__Import(const std::list<std::list<size_t>>& vals) {

		check_variables(this->Variable_names);

		if (vals.empty()) {
			throw 1; //found empty training set
		}

		this->Comb_allocated_size = vals.size();
		this->Comb_allocated = (size_t*)malloc(vals.front().size() * this->Comb_allocated_size * sizeof(size_t));
		size_t p = 0;
		size_t N_var = this->Variable_names.size();
		size_t k = 0;
		auto it_end = vals.end();
		list<size_t>::const_iterator it2, it2_end;
		for (auto it = vals.begin(); it != it_end; it++) {
			if (it->size() != N_var) {
				cout << "sample at pos " << k << " has inconsistent number of values" << endl;
				throw 0;
			}
			it2_end = it->end();
			for (it2 = it->begin(); it2 != it2_end; it2++) {
				this->Comb_allocated[p] = (*it2);
				p++;
			}
			k++;
		}

	}

	I_Potential::combinations::_Training_set_t::_Training_set_t(const std::list<_Training_set_t*>& to_merge) {

		if (to_merge.empty()) throw 0;

		size_t V = to_merge.front()->Variable_names.size(); 
		auto it = to_merge.begin();
		for (it; it != to_merge.end(); it++) {
			if ((*it)->Variable_names.size() != V) throw 1;
		}

		//check all training sets refer to the same set of variables
		struct _info {
			list<size_t>		pos;
			_Training_set_t*		source;
		};
		list<_info> info;
		it = to_merge.begin();
		info.push_back(_info());
		info.back().source = *it;
		for (size_t k = 0; k < (*it)->Variable_names.size(); k++)
			info.back().pos.push_back(k);
		it++;
		for (it; it != to_merge.end(); it++) {
			info.push_back(_info());
			info.back().source = *it;
			size_t p;
			for (auto itv = (*it)->Variable_names.begin(); itv != (*it)->Variable_names.end(); itv++) {
				p = 0;
				for (auto itv2 = to_merge.front()->Variable_names.begin(); itv2 != to_merge.front()->Variable_names.end(); itv2++) {
					if (*itv2 == *itv) {
						info.back().pos.push_back(p);
						break;
					}
					p++;
				}
			}

			if (info.back().pos.size() != V) throw 2; //at least one variable in the set was not found
		}

		this->Variable_names = to_merge.front()->Variable_names;
		this->Comb_allocated_size = 0;
		for (auto it = to_merge.begin(); it != to_merge.end(); it++)
			this->Comb_allocated_size += (*it)->Comb_allocated_size;
		this->Comb_allocated = (size_t*)malloc(this->Comb_allocated_size * sizeof(size_t));

		list<size_t>::iterator p, p_end;
		size_t c_tot = 0, c;
		size_t C;
		while (!info.empty()) {
			c = 0;
			p_end = info.front().pos.end();
			C = info.front().source->Comb_allocated_size;
			for (size_t k = 0; k < C; k++) {
				for (p = info.front().pos.begin(); p != p_end; p++) {
					this->Comb_allocated[c_tot] = info.front().source->Comb_allocated[c + *p];
					c_tot++;
				}
				c += V;
			}
			info.pop_front();
		}

	}

	void I_Potential::combinations::_Training_set_t::Print(const std::string& file_name) const {

		ofstream f(file_name);
		if (!f.is_open()) {
			f.close();
			throw 0; // inexistent file to print training set
		}

		size_t N_vars = this->Variable_names.size(), k;

		auto itn = this->Variable_names.begin();
		f << *itn;
		itn++;
		for (itn; itn != this->Variable_names.end(); itn++)
			f << " " << *itn;
		f << endl;

		size_t p = 0, v;
		for (k = 0; k < this->Comb_allocated_size; k++) {
			f << this->Comb_allocated[p];
			for (v = 0; v < N_vars; v++) {
				f << " " << this->Comb_allocated[p];
				p++;
			}
		}

		f.close();

	}

	I_Potential::combinations I_Potential::combinations::_Training_set_t::Get_as_combinations_list(const std::list<Categoric_var*>& variables_to_assume) const{

		combinations to_return;
		this->__Init_comb(to_return, variables_to_assume, this->Comb_allocated_size, this->Comb_allocated);
		return move(to_return);

	}

	I_Potential::combinations::_Training_set_t::subset::subset(_Training_set_t& set, const float& Sample_percentage):
	 Subject::Observer(&set.subset_subject, this) {

		this->pSource = &set;
		float perc = Sample_percentage;
		if (perc < 0.f) perc = 0.f;
		if (perc > 1.f) perc = 1.f;

		this->sampled_Comb_allocated_size = (size_t)floorf((float)this->pSource->Comb_allocated_size * perc);
		if (this->sampled_Comb_allocated_size == 0) this->sampled_Comb_allocated_size = 1;
		this->sampled_Comb_allocated = (size_t*)malloc(this->pSource->Variable_names.size() * this->sampled_Comb_allocated_size * sizeof(size_t));

		this->Resample();

	}

	void I_Potential::combinations::_Training_set_t::subset::Resample() {

		size_t s, S = this->pSource->Variable_names.size(), p;
		for (size_t k = 0; k < this->sampled_Comb_allocated_size; k++) {
			p = rand() % this->pSource->Comb_allocated_size;
			for (s = 0; s < S; s++) 
				this->sampled_Comb_allocated[k * S + s] = this->pSource->Comb_allocated[p * S + s];
		}

	}

	I_Potential::combinations I_Potential::combinations::_Training_set_t::subset::Get_as_combinations_list(const std::list<Categoric_var*>& variables_to_assume) const {

		combinations to_return;
		this->pSource->__Init_comb(to_return, variables_to_assume, this->sampled_Comb_allocated_size, this->sampled_Comb_allocated);
		return move(to_return);

	}

	void I_Potential::combinations::_Training_set_t::__Init_comb(combinations& combs, const list<Categoric_var*>& vars, const size_t& size, size_t* pcomb) const{

		check_variables(vars);
		if (vars.size() != this->Variable_names.size()) throw 0;
		//check the variable names match the ones owned by this object
		list<Categoric_var*> order_to_assume;
		this->__get_order(&order_to_assume , vars);

		combs.Entire_domain_was_allocated = false;
		combs.Variables = order_to_assume;
		combs.Size = size;
		combs.Combinations = pcomb;
		combs.free_Combinations = false;

	}

	void I_Potential::combinations::_Training_set_t::__get_order(std::list<Categoric_var*>* result, const std::list<Categoric_var*>& vars) const {

		result->clear();
		Categoric_var* temp = NULL;
		auto itn_end = this->Variable_names.end();
		list<Categoric_var*>::const_iterator it, it_end = vars.end();
		for (auto itn = this->Variable_names.begin(); itn != itn_end; itn++) {
			temp = NULL;
			for (it = vars.begin(); it != it_end; it++) {
				if ((*it)->Get_name().compare(*itn) == 0) {
					temp = *it;
					break;
				}
			}
			if (temp == NULL) throw 0; //at least one name was not found
			result->push_back(temp);
		}

	}

}

