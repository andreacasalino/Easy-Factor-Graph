/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Potential.h"
#include <string>
#include <sstream>
#include <ctime>
#include <float.h>
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;

namespace EFG {

	Categoric_var::Categoric_var(const size_t& size, const std::string& name) :Size(size), Name(name) {

		if (name.compare("") == 0)  throw 0; //empty name for Categoric variable is not valid

		if (size == 0) throw 1; //null size of categorical variable is not allowed

	}

	void check_all_vars_are_different(const list<Categoric_var*>& variables) {

		if (variables.empty()) throw 1;

		auto itv = variables.begin();
		auto itv_end = variables.end();
		itv++;
		list<Categoric_var*>::const_iterator it_cursor;
		for (itv; itv != itv_end; itv++) {
			for (it_cursor = variables.begin(); it_cursor != itv; it_cursor++) {
				if (*it_cursor == *itv) throw 0;
			}
		}

	}




	float I_Potential::max_in_distribution() const {

		auto Distr = this->Get_distr();
		auto it = Distr->begin();
		float max_val = (*it)->Get_val();
		float temp;
		it++;
		auto it_end = Distr->end();
		for (it; it != it_end; it++) {
			temp = (*it)->Get_val();
			if (temp > max_val) {
				max_val = temp;
			}
		}
		return max_val;

	}

	I_Potential::I_Potential(const I_Potential& to_copy) { 
		
		throw 0; //Potential of any kind cannot be copied like this

	}

	void print_distribution(std::ostream& f, const list<float>& to_print_val, const list<const size_t*>& to_print_comb, const size_t& comb_size) {

		auto it_c = to_print_comb.begin();
		auto it_v = to_print_val.begin();
		size_t k, K = to_print_val.size();
		size_t c;
		for (k = 1; k < K; k++) {
			for (c = 0; c < comb_size; c++)
				f << (*it_c)[c] << " ";
			f << *it_v << endl; 
			it_c++;
			it_v++;
		}
		for (c = 0; c < comb_size; c++)
			f << (*it_c)[c] << " ";
		f << *it_v;

	}
	void I_Potential::Print_distribution(std::ostream& f, const bool& print_entire_domain) const {

		list<float>			    to_print_val;
		list<const size_t*>		to_print_comb;

		if (print_entire_domain) {
			combinations comb(*this->Get_involved_var());
			vector<const I_Distribution_value*> v_temp;
			comb.Find_images_single_matches(&v_temp, *this);
			combinations::iterator it(comb);
			auto it_temp = v_temp.begin();
			while (it.is_not_at_end()) {
				if (*it_temp == NULL) to_print_val.push_back(0.f);
				else to_print_val.push_back((*it_temp)->Get_val());
				to_print_comb.push_back(*it);
				++it;
				it_temp++;
			}
		}
		else {
			auto distr = this->Get_distr();
			auto it = distr->begin();
			auto it_end = distr->end();
			for (it; it != it_end; it++) {
				to_print_val.push_back((*it)->Get_val());
				to_print_comb.push_back((*it)->Get_indeces());
			}
		}
		print_distribution(f,  to_print_val, to_print_comb, this->Get_involved_var()->size());

	}

	void I_Potential::Get_domain(std::vector<std::vector<size_t>>* domain) const {

		domain->clear();
		auto Dist = this->Get_distr();
		size_t k, K = this->Get_involved_var()->size();
		if (Dist->empty()) {
			domain->push_back(vector<size_t>());
			domain->back().reserve(K);
			for (k = 0; k < K; k++) domain->back().push_back(0);
		}
		else {
			domain->reserve(Dist->size());
			auto it = Dist->begin();
			auto it_end = Dist->end();
			const size_t* comb = NULL;
			for (it; it != it_end; it++) {
				comb = (*it)->Get_indeces();
				domain->push_back(vector<size_t>());
				domain->back().reserve(K);
				for (k = 0; k < K; k++) domain->back().push_back(comb[k]);
			}
		}

	}

	void I_Potential::Get_images(std::vector<float>* images) const {

		images->clear();
		auto Dist = this->Get_distr();
		if (Dist->empty()) images->push_back(0.f);
		else {
			images->reserve(Dist->size());
			auto it = Dist->begin();
			auto it_end = Dist->end();
			for (it; it != it_end; it++)
				images->push_back((*it)->Get_val());
		}

	}

	size_t I_Potential::combinations::Get_joint_domain_size(const list<Categoric_var*>& vars) {

		size_t S = 1;
		auto it_end = vars.end();
		for (auto it = vars.begin(); it != it_end; it++)
			S *= (*it)->size();
		return S;

	}

	I_Potential::combinations::combinations(combinations&& o) {

		this->Entire_domain_was_allocated = o.Entire_domain_was_allocated;
		this->Variables = o.Variables;
		this->Size = o.Size;
		this->Combinations = o.Combinations;
		o.Combinations = NULL;
		this->free_Combinations = o.free_Combinations;

	}

	I_Potential::combinations::combinations(const std::list<Categoric_var*>& variables) {

		if (variables.empty()) throw 0;
		check_all_vars_are_different(variables);
		this->Variables = variables;

		this->Size = Get_joint_domain_size(variables);
		this->Combinations = (size_t*)malloc(this->Size  * this->Variables.size() * sizeof(size_t));

		size_t k, K = variables.size();
		auto it_var = variables.begin();
		size_t Cycle_duration = 1, c, p, P = K * this->Size;
		size_t s, S;
		for (k = 0; k < K; k++) {
			p = k;
			S = (*it_var)->size();
			while (p < P) {
				for (s = 0; s < S; s++) {
					for (c = 0; c < Cycle_duration; c++) {
						this->Combinations[p] = s;
						p += K;
					}
				}
			}
			Cycle_duration *= (*it_var)->size();
			it_var++;
		}
		this->Entire_domain_was_allocated = true;
		this->free_Combinations = true;

	}

	I_Potential::combinations::combinations(const I_Potential& pot) {

		this->Variables = *pot.Get_involved_var();

		auto Distr = pot.Get_distr();
		size_t k, K = this->Variables.size();
		if (Distr->empty()) {
			//add only the combination <0,0,0,0, ... , 0>
			this->Size = 1;
			this->Combinations = (size_t*)malloc(K * sizeof(size_t));
			for (k = 0; k < K; k++)
				this->Combinations[k] = 0;
		}
		else {
			auto it = Distr->begin();
			auto it_end = Distr->end();
			size_t p = 0;
			this->Size = Distr->size();
			this->Combinations = (size_t*)malloc(K * this->Size * sizeof(size_t));
			const size_t* c = NULL;
			for (it; it != it_end; it++) {
				c = (*it)->Get_indeces();
				for (k = 0; k < K; k++)
					this->Combinations[p + k] = c[k];
				p += K;
			}
		}
		if(Distr->size() == Get_joint_domain_size(this->Variables))
			this->Entire_domain_was_allocated = true;
		this->free_Combinations = true;

	}

	I_Potential::combinations::combinations(const std::list<std::list<size_t>>& Combinations, const std::list<Categoric_var*>& variables) {

		if (Combinations.empty()) throw 0;
		if (variables.empty()) throw 1;
		check_all_vars_are_different(variables);
		this->Variables = variables;

		this->Size = Combinations.size();
		size_t k, K = this->Variables.size();
		this->Combinations = (size_t*)malloc(K * this->Size * sizeof(size_t));
		std::list<size_t>::const_iterator it_C;
		auto it_end = Combinations.end();
		size_t p = 0;
		for (auto it = Combinations.begin(); it != it_end; it++) {
			if (it->size() != K) {
				free(this->Combinations);
				throw 2;
			}
			it_C = it->begin();
			for (k = 0; k < K; k++) {
				this->Combinations[p] = *it_C;
				p++;
				it_C++;
			}
		}
		this->Entire_domain_was_allocated = false;
		this->free_Combinations = true;

	}

	I_Potential::combinations::iterator::iterator(const combinations& to_iterate) {

		this->Source = &to_iterate;
		this->Source_cursor = 0;
		this->Source_pos = 0;

	}

	const size_t* I_Potential::combinations::iterator::operator*() const {

		return &this->Source->Combinations[this->Source_cursor];

	}

	I_Potential::combinations::iterator& I_Potential::combinations::iterator::operator++() {

		this->Source_pos++;
		if (this->Source_pos > this->Source->Size) throw 0;

		this->Source_cursor += this->Source->Variables.size();
		return *this;

	}

	bool I_Potential::combinations::iterator::is_not_at_end() const {

		return (this->Source_pos < this->Source->Size);

	}

	void	Find_image_value(std::list<const I_Potential::I_Distribution_value*>* match_found, const std::list<I_Potential::I_Distribution_value*>* distr, 
		const vector<size_t>& pos_in_distr, const size_t* comb, const vector<size_t>& pos_in_comb ,const bool& arrest_at_first_found) {

		match_found->clear();
		auto it_end = distr->end();
		bool found;
		size_t k, K = pos_in_distr.size();
		const size_t* p1 = &pos_in_distr[0], * p2 = &pos_in_comb[0];
		const size_t* distr_comb;
		for (auto it = distr->begin(); it != it_end; it++) {
			found = true;
			distr_comb = (*it)->Get_indeces();
			for (k = 0; k < K; k++) {
				if (distr_comb[p1[k]] != comb[p2[k]]){
					found = false;
					break;
				}
			}
			if (found) {
				match_found->push_back(*it);
				if (arrest_at_first_found) return;
			}
		}

	};

	void   get_positions(vector<size_t>* pos_short, vector<size_t>* pos_long, const list<Categoric_var*>& vars_short, const list<Categoric_var*>& vars_long) {

		pos_short->reserve(vars_short.size());
		pos_long->reserve(vars_short.size());
		auto it_end = vars_short.end();
		size_t k,k2=0;
		list<Categoric_var*>::const_iterator it_long, it_long_end = vars_long.end();
		for (auto it = vars_short.begin(); it != it_end; it++) {
			pos_short->push_back(k2);
			it_long = vars_long.begin();
			k = 0;
			for (it_long; it_long != it_long_end; it_long++) {
				if (*it_long == *it) {
					pos_long->push_back(k);
					break;
				}
				k++;
			}
			k2++;
		}
		if (pos_short->size() != pos_long->size()) throw 0; //some variables were not found

	}

	void I_Potential::combinations::Find_images_single_matches(std::vector<const I_Distribution_value*>* result, const I_Potential& pot) const {

		auto pot_vars = pot.Get_involved_var();
		if (this->Variables.size() < pot_vars->size()) throw 0;
		result->clear();
		result->reserve(this->Size);
		const std::list<I_Potential::I_Distribution_value*>* Distr = pot.Get_distr();

		vector<size_t> pos_this, pos_pot;
		get_positions(&pos_pot , &pos_this , *pot_vars, this->Variables);

		//check for doing lazy research
		size_t k, P = this->Variables.size();
		if ((pot_vars->size() == this->Variables.size()) && this->Entire_domain_was_allocated) {
			size_t S = Get_joint_domain_size(this->Variables);

			if ((S == this->Size) && (S == Distr->size())) {
				bool proceed = true;
				for (k = 0; k < P; k++) {
					if (pos_this[k] != k) {
						proceed = false;
						break;
					}
				}
				if (proceed) {
					auto itd_end = Distr->end();
					for (auto itd = Distr->begin(); itd != itd_end; itd++)
						result->push_back(*itd);
					return;
				}
			}
		}

		size_t p = 0;
		std::list<const I_Potential::I_Distribution_value*> match;
		for (k = 0; k < this->Size; k++) {
			Find_image_value(&match, Distr, pos_pot, &this->Combinations[p], pos_this, true);
			if (match.empty()) result->push_back(NULL);
			else result->push_back(match.front());
			p += P;
		}

	}

	void I_Potential::combinations::Find_images_multiple_matches(std::vector<std::list<const I_Distribution_value*>>* result, const I_Potential& pot) const {

		auto pot_vars = pot.Get_involved_var();
		if (this->Variables.size() >= pot.Get_involved_var()->size()) throw 0;
		result->clear();
		result->reserve(this->Size);
		const std::list<I_Potential::I_Distribution_value*>* Distr = pot.Get_distr();

		vector<size_t> pos_this, pos_pot;
		get_positions(&pos_this, &pos_pot, this->Variables, *pot_vars);

		size_t p = 0, P = this->Variables.size(), k;
		for (k = 0; k < this->Size; k++) {
			result->push_back(list<const I_Distribution_value*>());
			Find_image_value(&result->back(), Distr, pos_pot, &this->Combinations[p], pos_this, false);
			p += P;
		}

	}

	size_t* alloc_combination(const size_t* to_clone, const size_t& Size) {

		size_t* temp = (size_t*)malloc(Size * sizeof(size_t));
		for (size_t k = 0; k < Size; k++)
			temp[k] = to_clone[k];
		return temp;

	}




	void Potential_Shape::__Register_to_observers() {

		for (auto it = this->Involved_var.begin(); it != this->Involved_var.end(); it++)
			this->var_observers.emplace_back((Subject*)(*it), (void*)this);

	}

	void check_substitution_consistency(const list<Categoric_var*>& original, const list<Categoric_var*>& novel) {

		if (original.size() != novel.size()) throw 0;

		auto it = original.begin();
		list<Categoric_var*> inserted;
		for (auto it2 = novel.begin(); it2 != novel.end(); it2++) {
			if ((*it)->size() != (*it2)->size())
				throw 1;
			it++;
		}

	}

	Potential_Shape::Potential_Shape(const list<Categoric_var*>& var_involved) {

		check_all_vars_are_different(var_involved);
		this->Involved_var = var_involved;
		this->__Register_to_observers();

	}

	void Import_line(const string& line, list<size_t>* indices, float* val) {

		indices->clear();
		istringstream iss(line);
		string temp;
		while (true) {
			if (iss.eof()) {
				break;
			}

			iss >> temp;
			indices->push_back((size_t)atoi(temp.c_str()));
		}

		indices->pop_back();
		*val = (float)atof(temp.c_str());

	};
	Potential_Shape::Potential_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read) :
		Potential_Shape(var_involved) {

		ifstream f(file_to_read);
		if (!f.is_open()) {
			f.close();
			throw 0; //invalid file for importing shape
		}

		string line;
		list<size_t> slices;
		float temp_val;
		size_t var_numb_expected = this->Involved_var.size();
		bool insert;
		while (!f.eof()) {
			getline(f, line);

			if (line.compare("") != 0) {
				Import_line(line, &slices, &temp_val);
				insert = true;
				if (var_numb_expected != slices.size()) {
#ifdef _DEBUG
					cout << "found invalid lines when importing shape : number of values mismatch";
#endif // DEBUG
					insert = false;
				}

				if (temp_val < 0.f) {
#ifdef _DEBUG
					cout << "found invalid lines when importing shape : negative balue";
#endif // DEBUG
					insert = false;
				}

				if (insert) this->Add_value(slices, temp_val);
			}
		}

		f.close();

	}

	Potential_Shape::Potential_Shape(const Potential_Shape& to_copy, const std::list<Categoric_var*>& var_involved) {

		check_all_vars_are_different(var_involved);
		check_substitution_consistency(to_copy.Involved_var , var_involved);		
		this->Involved_var = var_involved;

		if (!to_copy.Distribution.empty()) {
			size_t* temp;
			size_t L = this->Involved_var.size();
			auto itD_end = to_copy.Distribution.end();
			for (auto itD = to_copy.Distribution.begin(); itD != itD_end; itD++) {
				temp = alloc_combination((*itD)->Get_indeces(), L);
				this->Distribution.push_back(new Distribution_value_concrete(temp, (*itD)->Get_val()));
			}
		}

		this->__Register_to_observers();

	}

	Potential_Shape::Potential_Shape(const std::list<Categoric_var*>& var_involved, const bool& correlated_or_not) :
		Potential_Shape(var_involved) {

		if (var_involved.size() <= 1) throw 0; //simple correlation shapes must involve at least two variables
		
		auto it = var_involved.begin();
		size_t Size = (*it)->size(); it++;
		for (it; it != var_involved.end(); it++) {
			if ((*it)->size() != Size) throw 1; //variables in a simple correlating potential must have all same sizes
		}

		list<size_t> comb;
		if (correlated_or_not) {
			size_t kk;
			for (size_t k = 0; k < Size; k++) {
				comb.clear();
				for (kk = 0; kk < var_involved.size(); kk++)
					comb.push_back(k);
				this->Add_value(comb, 1.f);
			}
		}
		else {
			this->Set_ones();
			size_t k, L = var_involved.size();
			bool are_same;
			for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++) {
				are_same = true;
				for (k = 1; k < L; k++) {
					if ((*it)->Get_indeces()[k] != (*it)->Get_indeces()[0]) {
						are_same = false;
						break;
					}
				}

				if (are_same) {
					auto temp = new Distribution_value_concrete(alloc_combination((*it)->Get_indeces() , L) , 0.f);
					delete* it;
					*it = temp;
				}
			}
		}

	}

	Potential_Shape::Potential_Shape(const I_Potential& to_copy) :
		Potential_Shape(*to_copy.Get_involved_var()) {

		auto Dist = Get_distr_static(&to_copy);
		auto it_end = Dist->end();
		size_t S = this->Involved_var.size();
		for (auto it = Dist->begin(); it != it_end; it++) {
			this->Distribution.push_back(new Distribution_value_concrete(alloc_combination((*it)->Get_indeces(), S), (*it)->Get_val()));
		}

	}

	Potential_Shape::~Potential_Shape() {

		for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++)
			delete *it;

	}

	void Potential_Shape::__Set_Distribution_val(I_Distribution_value* to_set, const float& new_val) {

		static_cast<Distribution_value_concrete*>(to_set)->Set_val(new_val);

	}

	void Potential_Shape::__Check_add_value(const std::list<size_t>& indices) {

		size_t var_numb = this->Involved_var.size();
		if (indices.size() != var_numb) throw 0;

		bool match;
		size_t k;
		auto it_ind = indices.begin();
		for (auto it_d = this->Distribution.begin(); it_d != this->Distribution.end(); it_d++) {
			match = true;
			it_ind = indices.begin();
			for (k = 0; k < var_numb; k++) {
				if ((*it_d)->Get_indeces()[k] != *it_ind) {
					match = false;
					break;
				}
				it_ind++;
			}

			if (match) throw 1;
		}

		auto itV = this->Involved_var.begin();
		for (it_ind = indices.begin(); it_ind != indices.end(); it_ind++) {
			if (*it_ind >= (*itV)->size()) throw 2;
			itV++;
		}

	}

	void Potential_Shape::Add_value(const std::list<size_t>& new_indeces, const float& new_val) {

		if (this->subject.Get_observers_number() != 0) throw 1;
		if (new_val < 0.f) throw 0; //negative value not admitted for potential
		this->__Check_add_value(new_indeces);
		
		size_t K = this->Involved_var.size();
		size_t* temp = (size_t*)malloc(sizeof(size_t)*K);
		auto it_ind = new_indeces.begin();
		for (size_t k = 0; k < K; k++) {
			temp[k] = *it_ind;
			it_ind++;
		}

		this->Distribution.push_back(new Distribution_value_concrete(temp, new_val));

	}

	void Potential_Shape::__Alloc_entire_domain(const float& val_to_use_4_entire_domain) {

		size_t S = 1;
		for (auto it = this->Involved_var.begin(); it != this->Involved_var.end(); it++)
			S *= (*it)->size();
		if (S == this->Distribution.size()) return;

		for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++)
			delete* it;
		this->Distribution.clear();

		combinations temp(this->Involved_var);
		size_t* c_temp = NULL;
		size_t K = this->Involved_var.size();
		combinations::iterator it_temp(temp);
		while (it_temp.is_not_at_end()) {
			c_temp = alloc_combination(*it_temp, K);
			this->Distribution.push_back(new Distribution_value_concrete(c_temp, val_to_use_4_entire_domain));
			++it_temp;
		}

	}

	void Potential_Shape::Set_ones() {

		if (this->subject.Get_observers_number() != 0) throw 1;

		this->__Alloc_entire_domain(1.f);

	}

#define RAND_MAX_FLOAT float(RAND_MAX)
	void Potential_Shape::Set_random() {

		if (this->subject.Get_observers_number() != 0) throw 1;
		//srand((unsigned int)time(NULL));

		this->__Alloc_entire_domain(0.f);
		auto itD_end = this->Distribution.end();
		size_t L = this->Involved_var.size();
		for (auto itD = this->Distribution.begin(); itD != itD_end; itD++)
			this->__Set_Distribution_val(*itD, (float)rand() / RAND_MAX_FLOAT);

	}

	void Potential_Shape::Normalize_distribution() {

		if (this->subject.Get_observers_number() != 0) throw 1;
		if (!this->Distribution.empty()) {
			float Rescale = 0.f, temp = 0.f;
			auto it = this->Distribution.begin();
			auto it_end = this->Distribution.end();
			for (it; it != it_end; it++) {
				temp = (*it)->Get_val();
				if (temp > Rescale) Rescale = temp;
			}
			if (Rescale == 0.f) return;
			Rescale = 1.f / Rescale;

			size_t L = this->Involved_var.size();
			for (it = this->Distribution.begin();
				it != it_end; it++)
				this->__Set_Distribution_val(*it , Rescale * (*it)->Get_val());
		}

	}

	void Potential_Shape::Substitute_variables(const std::list<Categoric_var*>& new_var) {

		if (this->subject.Get_observers_number() != 0) throw 1;
		check_all_vars_are_different(new_var);
		check_substitution_consistency(this->Involved_var, new_var);
		this->Involved_var = new_var;

		auto it_end = this->Involved_var.end();
		auto it_ob = this->var_observers.begin();
		for (auto it = this->Involved_var.begin(); it != it_end; it++) {
			it_ob->change_observed(*it);
			it_ob++;
		}

	}


 

	class Potential_Exp_Shape::Distribution_exp_value : public I_Potential::I_Distribution_value {
	public:
		Distribution_exp_value(Distribution_value_concrete* to_wrap, float* weight) :wrapped(to_wrap), w(weight) {};
		~Distribution_exp_value() { delete this->wrapped; };

		virtual float		  Get_val() const { return expf(*w * this->wrapped->Get_val()); };
		virtual const size_t* Get_indeces() const { return this->wrapped->Get_indeces(); };
	protected:
	// data
		float*						 w;
		Distribution_value_concrete* wrapped;
	};

	Potential_Exp_Shape::~Potential_Exp_Shape() {

		this->shape_observer->detach();
		for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++)
			delete *it;
		delete this->shape_observer;

	}

	void Potential_Exp_Shape::Wrap(Potential_Shape* shape) {

		Subject* shape_sub = shape->Cast_to_Subject();
		if (shape_sub->Get_observers_number() != 0) throw 0;
		this->shape_observer = new Subject::Observer(shape_sub, this);
		this->pwrapped = shape;
		
		combinations comb(*shape->Get_involved_var());
		vector<const I_Distribution_value*> vals;
		comb.Find_images_single_matches(&vals, *shape);
		auto it = vals.begin();
		size_t S = this->Get_involved_var()->size();
		combinations::iterator it_comb(comb);
		Distribution_value_concrete* val = NULL;
		while (it_comb.is_not_at_end()) {
			if(*it == NULL)
				val = new Distribution_value_concrete(alloc_combination(*it_comb, S), 0.f);
			else
				val = new Distribution_value_concrete(alloc_combination(*it_comb, S), (*it)->Get_val());
			this->Distribution.push_back(new Distribution_exp_value(val , &this->mWeight));
			it++;
			++it_comb;
		}

	}

	Potential_Exp_Shape::Potential_Exp_Shape(Potential_Shape& shape, const float& w) :
		I_Potential_Decorator(NULL), mWeight(w), shape_observer(NULL) {

		this->Wrap(&shape);

	};

	Potential_Exp_Shape::Potential_Exp_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read, const float& w) :
		I_Potential_Decorator(NULL), mWeight(w), shape_observer(NULL) {

		Potential_Shape* temp = new Potential_Shape(var_involved, file_to_read);
		this->Wrap(temp);

	}

	Potential_Exp_Shape::Potential_Exp_Shape(const Potential_Exp_Shape& to_copy, const std::list<Categoric_var*>& var_involved) :
		I_Potential_Decorator(NULL), mWeight(to_copy.mWeight), shape_observer(NULL) {

		auto new_shape = new Potential_Shape(*to_copy.pwrapped, var_involved);
		this->Wrap(new_shape);

	};

	Potential_Exp_Shape::weigth_modifier::weigth_modifier(Potential_Exp_Shape& involved_pot) :
		Observer(&involved_pot.subject_w_hndlr, (void*)(&involved_pot)) {

		this->pWeight = &involved_pot.mWeight;

	};





	void get_all_equals(vector<float>* vals, const size_t& Size) {

		vals->reserve(Size);
		for (size_t k = 0; k < Size; k++)
			vals->push_back(1.f);

	};
	Potential::Potential(const std::list<Potential*>& potential_to_merge, const bool& use_sparse_format) : I_Potential_Decorator(NULL) {

		if (potential_to_merge.empty())  throw 0; //empty set to merge
		
		combinations* comb = NULL;
		if (use_sparse_format)
			comb = new combinations(*potential_to_merge.front());
		else
			comb = new combinations(*potential_to_merge.front()->Get_involved_var());

		vector<float> v_cumulated;
		vector<const I_Distribution_value*> v_temp;
		get_all_equals(&v_cumulated, comb->get_number_of_combinations());
		vector<const I_Distribution_value*>::iterator itc;
		vector<float>::iterator itv, itv_end = v_cumulated.end();
		auto it_end = potential_to_merge.end();
		for (auto it = potential_to_merge.begin(); it != it_end; it++) {
			comb->Find_images_single_matches(&v_temp , **it);
			itc = v_temp.begin();
			for (itv = v_cumulated.begin(); itv != itv_end; itv++) {
				if (*itc == NULL) *itv *= 0.f;
				else *itv *= (*itc)->Get_val();
				itc++;
			}
		}

		this->pwrapped = new Potential_Shape(*potential_to_merge.front()->Get_involved_var());
		auto Dist = Get_distr_static(this->pwrapped);
		combinations::iterator it_comb(*comb);
		size_t S = potential_to_merge.front()->Get_involved_var()->size();
		for (itv = v_cumulated.begin(); itv != itv_end; itv++) {
			if (*itv != 0.f) 
				Dist->push_back(new Distribution_value_concrete(alloc_combination(*it_comb, S), *itv));
			++it_comb;
		}
		delete comb;

	};

	Potential::Potential(const std::list<size_t>& val_observed, const std::list<Categoric_var*>& var_observed, Potential& pot_to_reduce) : I_Potential_Decorator(NULL) {

		if (val_observed.size() != var_observed.size()) throw 0;// val observed mismatch with var observed
		
		if (val_observed.size() >= pot_to_reduce.Get_involved_var()->size() ) throw 1; // at least one non observed variable must exist
		
		combinations comb( val_observed , var_observed);
		std::vector<std::list<const I_Distribution_value*>> matching;
		comb.Find_images_multiple_matches(&matching, pot_to_reduce);
		list<size_t> pos_remaining;
		list<Categoric_var*> var_remaining;
		bool is_not_observed;
		auto vars = pot_to_reduce.Get_involved_var();
		size_t k = 0;
		list<Categoric_var*>::const_iterator ito, ito_end = var_observed.end();
		for (auto it = vars->begin(); it != vars->end(); it++) {
			is_not_observed = true;
			for (ito = var_observed.begin(); ito != ito_end; ito++) {
				if (*ito == *it) {
					is_not_observed = false;
					break;
				}
			}
			if (is_not_observed) {
				var_remaining.push_back(*it);
				pos_remaining.push_back(k);
			}
			k++;
		}

		this->pwrapped = new Potential_Shape(var_remaining);
		auto Dist = Get_distr_static(this->pwrapped);
		size_t* comb_remaining = NULL;
		auto itM = matching.front().begin();
		auto itM_end = matching.front().end();
		size_t r, R = pos_remaining.size();
		list<size_t>::iterator it_r;
		const size_t* comb_original = NULL;
		for (itM; itM != itM_end; itM++) {
			comb_remaining = (size_t*)malloc(R * sizeof(size_t));
			comb_original = (*itM)->Get_indeces();
			it_r = pos_remaining.begin();
			for (r = 0; r < R; r++) {
				comb_remaining[r] = comb_original[*it_r];
				it_r++;
			}
			Dist->push_back(new Distribution_value_concrete(comb_remaining, (*itM)->Get_val()));
		}

	}

	void Potential::Get_marginals(std::vector<float>* prob_distr) const {

		prob_distr->clear();
		auto vars = this->Get_involved_var();
		combinations comb(*vars);
		prob_distr->reserve(comb.get_number_of_combinations());
		vector<const I_Distribution_value*> match;
		comb.Find_images_single_matches(&match, *this->pwrapped);
		size_t K = match.size();
		float Z = 0.f;
		for (size_t k = 0; k < K; k++) {
			if (match[k] == NULL) prob_distr->push_back(0.f);
			else {
				prob_distr->push_back(match[k]->Get_val());
				Z += prob_distr->back();
			}
		}

		vector<float>::iterator itP, itP_end = prob_distr->end();
		if (Z < 1e-8) {
			float temp = 1.f / (float)prob_distr->size();
			for (itP = prob_distr->begin(); itP != itP_end; itP++)
				*itP = temp;
		}
		else {
			Z = 1.f / Z;
			for (itP = prob_distr->begin(); itP != itP_end; itP++)
				*itP *= Z;
		}

	}

	void Potential::clone_distribution(Potential_Shape& shape) const {

		shape.Substitute_variables(*this->Get_involved_var()); //inside this method is checked that the shape is not observed by anyone
		auto Dist = Get_distr_static(&shape);
		for (auto it = Dist->begin(); it != Dist->end(); it++)
			delete* it;
		Dist->clear();

		const std::list<I_Distribution_value*>* Dist_this = Get_distr_static(this);
		size_t K = this->Get_involved_var()->size();
		auto it = Dist_this->begin();
		auto it_end = Dist_this->end();
		for (it; it != it_end; it++)
			Dist->push_back(new Distribution_value_concrete(alloc_combination((*it)->Get_indeces(), K), (*it)->Get_val()));

	}

}