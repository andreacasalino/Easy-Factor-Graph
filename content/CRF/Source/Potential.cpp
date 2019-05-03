#include "../Header/Potential.h"
#include <string>
#include <sstream>
#include <ctime>
using namespace std;

namespace Segugio {

	Categoric_var::Categoric_var(const size_t& size, const std::string& name):Size(size), Name(name) {

		if (name.compare("") == 0) {
			system("ECHO empty name for Categoric variable is not valid");
			abort();
		}

		if (size == 0) {
			system("ECHO null size of categorical variable is not allowed");
			abort();
		}

	}

	const float& Categoric_domain::operator[](const size_t& pos) {

#ifdef _DEBUG
		if (pos > this->Size) {
			system("ECHO out of bounds");
			abort();
		}
#endif 

		return this->Domain[pos];

	}





	struct Distribution_value : public I_Potential::I_Distribution_value {
		friend struct Distribution_exp_value;

		Distribution_value(size_t* ind, const float& v = 0.f) : val(v) { this->indices = ind; };
		~Distribution_value();

		void	Set_val(const float& v) { this->val = v; };
		void	Get_val(float* result) { *result = val; };
		size_t* Get_indeces() { return this->indices; };
	protected:
	// data
		size_t* indices; //this distribution value refers to the combination of indeces expressed by this variable
		float   val;
	};

	Distribution_value::~Distribution_value() {
		free(indices);
	};

	struct Distribution_exp_value : public I_Potential::I_Distribution_value {
		Distribution_exp_value(Distribution_value* to_wrap, float* weight) :wrapped(to_wrap), w(weight) {};
		~Distribution_exp_value();

		void	Set_val(const float& v) { this->wrapped->val = v; };
		void	Get_val(float* result) { *result = expf(this->wrapped->val * *this->w); };
		size_t* Get_indeces() { return this->wrapped->indices; };
	protected:
	// data
		float*   w;
		Distribution_value* wrapped;
	};

	Distribution_exp_value::~Distribution_exp_value() {
		delete wrapped;
	};

	void I_Potential::Get_entire_domain(list<size_t*>* domain, const list<Categoric_var*>& Vars_in_domain) {

		domain->clear();

		size_t var_num = Vars_in_domain.size(), kV = 1, k;
		auto itV = Vars_in_domain.begin();
		size_t* new_val;
		for (k = 0; k < (*itV)->size(); k++) {
			new_val = (size_t*)malloc(sizeof(size_t)*var_num);
			new_val[0] = k;
			domain->push_back(new_val);
		}
		itV++;

		size_t k2, k3, Domain_size;
		list<size_t*>::iterator it_domain;
		for (itV; itV != Vars_in_domain.end(); itV++) {
			it_domain = domain->begin();
			Domain_size = domain->size();
			for (k2 = 0; k2 < Domain_size; k2++) {

				for (k = 1; k < (*itV)->size(); k++) {
					new_val = (size_t*)malloc(sizeof(size_t)*var_num);
					for (k3 = 0; k3 < kV; k3++)
						new_val[k3] = (*it_domain)[k3];
					new_val[kV] = k;
					domain->push_back(new_val);
				}
				(*it_domain)[kV] = 0;

				it_domain++;
			}
			kV++;
		}

	}

	void I_Potential::Get_entire_domain(std::list<std::list<size_t>>* domain, const std::list<Categoric_var*>& Vars_in_domain) {

		list<size_t*> domain_malloc;
		I_Potential::Get_entire_domain(&domain_malloc, Vars_in_domain);

		domain->clear();
		auto it = domain_malloc.begin();
		size_t k, K = Vars_in_domain.size();
		for (it; it != domain_malloc.end(); it++) {
			domain->push_back(list<size_t>());
			for (k = 0; k < K; k++)
				domain->back().push_back((*it)[k]);
		}

		for (it = domain_malloc.begin(); it != domain_malloc.end(); it++)
			free(*it);

	}

	void __find_Comb_in_distribution(list<I_Potential::I_Distribution_value*>* result, const list<size_t*>& comb, const list<size_t>& comb_pos, 
		                        const list<I_Potential::I_Distribution_value*>* distr, const list<size_t>& distr_pos, const bool& arrest_at_first_found) {

		result->clear();

		list<size_t>::const_iterator it_comb_pos, it_distr_pos;
		list<I_Potential::I_Distribution_value*>::const_iterator it_distr;
		bool match;
		bool found;
		for (auto it_comb = comb.begin(); it_comb != comb.end(); it_comb++) {
			found = false;
			for (it_distr = distr->begin(); it_distr != distr->end(); it_distr++) {
				match = true;
				it_comb_pos = comb_pos.begin();
				for (it_distr_pos = distr_pos.begin(); it_distr_pos != distr_pos.end(); it_distr_pos++) {
					if ((*it_distr)->Get_indeces()[*it_distr_pos] != (*it_comb)[*it_comb_pos]) {
						match = false;
						break;
					}
					it_comb_pos++;
				}

				if (match) {
					result->push_back(*it_distr);
					found = true;
					if (arrest_at_first_found) break;
				}
			}

			if (!found) {
				result->push_back(NULL);
			}
		}

	}

	void I_Potential::Find_Comb_in_distribution(list<I_Distribution_value*>* result, const list<size_t*>& comb_to_search, 
		                                        const list<Categoric_var*>& comb_to_search_var_order, I_Potential* pot) {

		list<size_t> comb_pos, distr_pos;

		list<Categoric_var*>::const_iterator it_comb_to_search_var_order;
		auto vars = I_Potential::Get_involved_var(pot);
		size_t k = 0, k2;
		for (auto it_var = vars->begin(); it_var != vars->end(); it_var++) {
			distr_pos.push_back(k);
			k2 = 0;
			for (it_comb_to_search_var_order = comb_to_search_var_order.begin(); 
				it_comb_to_search_var_order != comb_to_search_var_order.end(); it_comb_to_search_var_order++) {
				if (*it_comb_to_search_var_order == *it_var) {
					comb_pos.push_back(k2);
					break;
				}
				k2++;
			}
			k++;
		}

		if (comb_pos.size() != distr_pos.size()) {
			system("ECHO soma variables were not found when searching for matchings");
			abort();
		}

		__find_Comb_in_distribution(result, comb_to_search, comb_pos, I_Potential::Get_distr(pot), distr_pos, true);

	}

	void I_Potential::Find_Comb_in_distribution(std::list<I_Distribution_value*>* result, size_t* partial_comb_to_search,
		const std::list<Categoric_var*>& partial_comb_to_search_var_order, I_Potential* pot) {

		list<size_t> comb_pos, distr_pos;
		size_t k = 0, k2;
		auto vars = I_Potential::Get_involved_var(pot);
		list<Categoric_var*>::iterator it_vars;
		for (auto it_comb = partial_comb_to_search_var_order.begin();
			it_comb != partial_comb_to_search_var_order.end(); it_comb++) {
			comb_pos.push_back(k);
			k2 = 0;
			for (it_vars = vars->begin(); it_vars != vars->end(); it_vars++) {
				if (*it_vars == *it_comb) {
					distr_pos.push_back(k2);
					break;
				}
				k2++;
			}

			k++;
		}

		if (comb_pos.size() != distr_pos.size()) {
			system("ECHO soma variables were not found when searching for matchings");
			abort();
		}

		__find_Comb_in_distribution(result, { partial_comb_to_search }, comb_pos, I_Potential::Get_distr(pot), distr_pos, false);

	}

	void I_Potential::Find_Comb_in_distribution(std::list<float>* result,
		const std::list<size_t*>& comb_to_search, const std::list<Categoric_var*>& comb_to_search_var_order) {

		list<I_Distribution_value*> temp;
		this->Find_Comb_in_distribution(&temp, comb_to_search, comb_to_search_var_order, this);
		result->clear();
		float temp_fl;
		for (auto it = temp.begin(); it != temp.end(); it++) {
			if (*it == NULL) result->push_back(0.f);
			else {
				(*it)->Get_val(&temp_fl);
				result->push_back(temp_fl);
			}
		}

	}







	Potential_Shape::Potential_Shape(const list<Categoric_var*>& var_involved) {

		list<Categoric_var*>::iterator itV2;
		for (auto itV = var_involved.begin(); itV != var_involved.end(); itV++) {
			for (itV2 = this->Involved_var.begin(); itV2 != this->Involved_var.end(); itV2++) {
				if (*itV2 == *itV) {
					system("ECHO all variables in a shape function must be different");
					abort();
				}
			}

			this->Involved_var.push_back(*itV);
		}

	}

	Potential_Shape::Potential_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read):
		Potential_Shape(var_involved) {

		this->Import(file_to_read);

	}

	Potential_Shape::~Potential_Shape() {

		for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++)
			delete *it;

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

	void Potential_Shape::Import(const std::string& file_to_read) {

		ifstream f(file_to_read);
		if (!f.is_open()) {
			system("ECHO I_Potential::Import: invalid file");
			abort();
		}

		string line;
		list<size_t> slices;
		float temp_val;
		size_t var_numb_expected = this->Involved_var.size();
		while (!f.eof()) {
			getline(f, line);
			Import_line(line, &slices, &temp_val);
			if (var_numb_expected != slices.size()) {
				system("ECHO I_Potential::Import: mismatch");
				abort();
			}

			if (temp_val < 0.f) {
				system("ECHO negative value not admitted for potential");
				abort();
			}

			this->Add_value(slices, temp_val);

		}

		f.close();

	}

	void Potential_Shape::Check_add_value(const std::list<size_t>& indices) {

		size_t var_numb = this->Involved_var.size();
		if (indices.size() != var_numb) {
			system("ECHO You tried to insert a new value with wrong dimension");
			abort();
		}

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

			if (match) {
				system("ECHO You tried to add a value already inserted in a potential");
				abort();
			}
		}

		auto itV = this->Involved_var.begin();
		for (it_ind = indices.begin(); it_ind != indices.end(); it_ind++) {
			if (*it_ind >= (*itV)->size()) {
				system("ECHO indice out of bound");
				abort();
			}

			itV++;
		}

	}

	void Potential_Shape::Add_value(const std::list<size_t>& new_indeces, const float& new_val) {

		if (new_val < 0.f) {
			system("ECHO negative value not admitted for potential");
			abort();
		}

		this->Check_add_value(new_indeces);

		size_t* temp = (size_t*)malloc(sizeof(size_t)*this->Involved_var.size());
		auto it_ind = new_indeces.begin();
		for (size_t k = 0; k < this->Involved_var.size(); k++) {
			temp[k] = *it_ind;
			it_ind++;
		}

		this->Distribution.push_back(new Distribution_value(temp));
		this->Distribution.back()->Set_val(new_val);

	}

	void Potential_Shape::Set_ones() {

		for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++)
			delete *it;
		this->Distribution.clear();

		list<size_t*> domain;
		Get_entire_domain(&domain, this->Involved_var);
		for (auto itD = domain.begin(); itD != domain.end(); itD++) {
			this->Distribution.push_back(new Distribution_value(*itD));
			this->Distribution.back()->Set_val(1.f);
		}

	}

	void Potential_Shape::Set_random(const float zeroing_threashold) {

		//srand((unsigned int)time(NULL));

		for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++)
			delete *it;
		this->Distribution.clear();

		list<size_t*> domain;
		Get_entire_domain(&domain, this->Involved_var);
		for (auto itD = domain.begin(); itD != domain.end(); itD++) {
			if ((float)rand() / (float)RAND_MAX < zeroing_threashold) {
				this->Distribution.push_back(new Distribution_value(*itD));
				this->Distribution.back()->Set_val((float)rand() / (float)RAND_MAX);
			}
			else  free(*itD);
		}

	}

	void Potential_Shape::Normalize_distribution() {

		if (!this->Distribution.empty()) {
			float Rescale = 0.f, temp;
			auto it = this->Distribution.begin();
			for (it; it != this->Distribution.end(); it++) {
				(*it)->Get_val(&temp);
				if (temp > Rescale) Rescale = temp;
			}
			if (Rescale == 0.f) return;
			Rescale = 1.f / Rescale;

			for (it = this->Distribution.begin();
				it != this->Distribution.end(); it++) {
				(*it)->Get_val(&temp);
				(*it)->Set_val(Rescale * temp);
			}
		}

	}






	Potential_Exp_Shape::~Potential_Exp_Shape() {

		for (auto it = this->Distribution.begin(); it != this->Distribution.end(); it++)
			delete *it;

	}

	void Potential_Exp_Shape::Wrap(Potential_Shape* shape) {

		this->pwrapped = shape;

		list<size_t*> entire_domain;
		Get_entire_domain(&entire_domain, *I_Potential::Get_involved_var(shape));
		
		list<I_Distribution_value*> _distribution;
		shape->Find_Comb_in_distribution(&_distribution, entire_domain, *I_Potential::Get_involved_var(shape), shape);
		auto it_domain = entire_domain.begin();
		float temp;
		Distribution_value* temp_pt;
		for (auto it = _distribution.begin(); it != _distribution.end(); it++) {
			temp_pt = new Distribution_value(*it_domain);
			if (*it == NULL) temp_pt->Set_val(0.f);
			else {
				(*it)->Get_val(&temp);
				temp_pt->Set_val(temp);
			}
			this->Distribution.push_back(new Distribution_exp_value(temp_pt, &this->mWeight));
			it_domain++;
		}

	}

	Potential_Exp_Shape::Potential_Exp_Shape(Potential_Shape* shape, const float& w):
		I_Potential_Decorator(NULL), mWeight(w) {

		this->Wrap(shape);

	};

	Potential_Exp_Shape::Potential_Exp_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read, const float& w):
		I_Potential_Decorator(NULL), mWeight(w) {

		Potential_Shape* temp = new Potential_Shape(var_involved);
		temp->Import(file_to_read);
		this->Wrap(temp);

	}





	Potential::Potential(const std::list<Potential*>& potential_to_merge, const bool& use_sparse_format) : I_Potential_Decorator(NULL) {

		if (potential_to_merge.empty()) {
			return;
			system("ECHO empty set to merge");
			abort();
		}

		list<Categoric_var*>* front_vars = I_Potential::Get_involved_var(potential_to_merge.front());
		list<list<I_Distribution_value*>> Distributions_to_merge;
		list<size_t*> val_to_search;
		if (use_sparse_format) {
			size_t* temp_malloc;
			//copy all the values in front distribution
			size_t k, N_var = front_vars->size();
			auto Distribution_front = I_Potential::Get_distr(potential_to_merge.front());
			for (auto itD = Distribution_front->begin(); itD != Distribution_front->end(); itD++) {
				temp_malloc = (size_t*)malloc(sizeof(size_t)*N_var);
				for (k = 0; k < N_var; k++)
					temp_malloc[k] = (*itD)->Get_indeces()[k];
				val_to_search.push_back(temp_malloc);
			}
		}
		else {
			Get_entire_domain(&val_to_search, *front_vars);
		}

		for (auto itP = potential_to_merge.begin(); itP != potential_to_merge.end(); itP++) {
			Distributions_to_merge.push_back(list<I_Distribution_value*>());
			Find_Comb_in_distribution(&Distributions_to_merge.back(), val_to_search, *front_vars, *itP);
		}



		//build the merged distribution

		Potential_Shape* shape_to_create = new Potential_Shape(*front_vars);
		auto shape_Distr = I_Potential::Get_distr(shape_to_create);

		bool match_in_all;
		float product, temp;
		auto it_Distributions = Distributions_to_merge.begin();
		while (!Distributions_to_merge.front().empty()) {
			match_in_all = true;

			product = 1.f;
			for (it_Distributions = Distributions_to_merge.begin(); it_Distributions != Distributions_to_merge.end(); it_Distributions++) {
				if (it_Distributions->front() == NULL) {
					match_in_all = false;
					break;
				}

				it_Distributions->front()->Get_val(&temp);
				product *= temp;
			}

			if (match_in_all) {
				shape_Distr->push_back(new Distribution_value(val_to_search.front()));
				shape_Distr->back()->Set_val(product);
			}
			else {
				if (use_sparse_format) free(val_to_search.front());
				else {
					shape_Distr->push_back(new Distribution_value(val_to_search.front()));
					shape_Distr->back()->Set_val(0.f);
				}
			}

			
			val_to_search.pop_front();
			for (it_Distributions = Distributions_to_merge.begin(); it_Distributions != Distributions_to_merge.end(); it_Distributions++)
				it_Distributions->pop_front();
		}

		this->pwrapped = shape_to_create;

	};

	Potential::Potential(const std::list<size_t>& val_observed, const std::list<Categoric_var*>& var_observed, Potential* pot_to_reduce) : I_Potential_Decorator(NULL) {

#ifdef _DEBUG
		if (val_observed.size() != var_observed.size()) {
			system("ECHO val observed mismatch with var observed");
			abort();
		}

		if (val_observed.size() >= I_Potential::Get_involved_var(pot_to_reduce)->size() ) {
			system("ECHO at least one non observed variable must exist");
			abort();
		}
#endif // _DEBUG	

		size_t* index_to_find = (size_t*)malloc(sizeof(size_t)*var_observed.size());
		auto val_ob = val_observed.begin();
		for (size_t k = 0; k < var_observed.size(); k++) {
			index_to_find[k] = *val_ob;
			val_ob++;
		}

		list<size_t>			 pos_to_remain;
		list<Categoric_var*>	 var_to_remain;
		auto it_obs = var_observed.begin();
		bool is_obs;
		size_t k = 0;
		auto pot_to_reduce_vars = I_Potential::Get_involved_var(pot_to_reduce);
		for (auto itV = pot_to_reduce_vars->begin(); itV != pot_to_reduce_vars->end(); itV++) {
			is_obs = false;
			for (it_obs = var_observed.begin(); it_obs != var_observed.end(); it_obs++) {
				if (*it_obs == *itV) {
					is_obs = true;
					break;
				}
			}

			if (!is_obs) {
				pos_to_remain.push_back(k);
				var_to_remain.push_back(*itV);
			}
			k++;
		}

		list<I_Distribution_value*> vals;
		Find_Comb_in_distribution(&vals, index_to_find, var_observed, pot_to_reduce);
		free(index_to_find);

		Potential_Shape* shape_to_create = new Potential_Shape(var_to_remain);
		auto distr_to_create = I_Potential::Get_distr(shape_to_create);

		if (vals.front() != NULL) {
			size_t Remain_size = var_to_remain.size();
			auto it_remain = pos_to_remain.begin();
			float temp;
			size_t* temp_malloc;
			for (auto itD = vals.begin(); itD != vals.end(); itD++) {
				temp_malloc = (size_t*)malloc(sizeof(size_t) * Remain_size);
				k = 0;
				for (it_remain = pos_to_remain.begin(); it_remain != pos_to_remain.end(); it_remain++) {
					temp_malloc[k] = (*itD)->Get_indeces()[*it_remain];
					k++;
				}

				distr_to_create->push_back(new Distribution_value(temp_malloc));
				(*itD)->Get_val(&temp);
				distr_to_create->back()->Set_val(temp);
			}
		}

		this->pwrapped = shape_to_create;

	}

	void Potential::Get_marginals(std::list<float>* prob_distr) {

		prob_distr->clear();

		auto distr = this->Get_distr();
#ifdef _DEBUG
		if (distr->empty()) {
			system("ECHO empty distribution not possible");
			abort();
		}
#endif // _DEBUG

		float Z = 0.f;
		for (auto itD = distr->begin(); itD != distr->end(); itD++) {
			prob_distr->push_back(float());
			(*itD)->Get_val(&prob_distr->back());
			Z += prob_distr->back();
		}


		if (Z < 1e-8) {
			float temp = 1.f / (float)distr->size();
			for (auto itP = prob_distr->begin(); itP != prob_distr->end(); itP++)
				*itP = temp;
		}
		else {
			Z = 1.f / Z;
			for (auto itP = prob_distr->begin(); itP != prob_distr->end(); itP++)
				*itP *= Z;
		}

	}





	Message_Unary::Message_Unary(Categoric_var* var_involved) {

		Potential_Shape* ones = new Potential_Shape({ var_involved });
		ones->Set_ones();
		this->pwrapped = ones;

	}

	void eval_diff(float* diff, list<I_Potential::I_Distribution_value*>* F1, list<I_Potential::I_Distribution_value*>* F2) {

		if (F1->size() != F2->size()) {
			*diff = FLT_MAX;
			return;
		}

		*diff = 0.f;
		float v1, v2;
		auto it2 = F2->begin();
		for (auto it1 = F1->begin(); it1 != F1->end(); it1++) {
			(*it1)->Get_val(&v1);
			(*it2)->Get_val(&v2);
			*diff += abs(v1 - v2);
			it2++;
		}

	}

	Message_Unary::Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP) {

		float fake_diff;
		this->Update(&fake_diff, binary_to_merge, potential_to_merge, Sum_or_MAP);

	}

	void Message_Unary::Update(float* diff_to_previous, Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP) {

		if (potential_to_merge.empty()) {
			system("ECHO Message::Update: wrong method for managing an empty set of unary to merge");
			abort();
		}

		Potential* unary_union = NULL;
		if (potential_to_merge.size() == 1)  unary_union = potential_to_merge.front();
		else unary_union = new Potential(potential_to_merge);

		auto merged_pot = this->merge_binary_and_unary(binary_to_merge, unary_union, Sum_or_MAP);
		merged_pot->Normalize_distribution();

		if (potential_to_merge.size() != 1) delete unary_union;

		if (pwrapped == NULL)
			*diff_to_previous = FLT_MAX;
		else {
			eval_diff(diff_to_previous, this->I_Potential::Get_distr(this->pwrapped), this->I_Potential::Get_distr(merged_pot));
			delete this->pwrapped;
		}
		this->pwrapped = merged_pot;

	}

	Message_Unary::Message_Unary(Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP) {

		float fake_diff;
		this->Update(&fake_diff, binary_to_merge, var_to_marginalize, Sum_or_MAP);

	}

	void Message_Unary::Update(float* diff_to_previous, Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP) {

		Message_Unary unary_temp(var_to_marginalize);

		auto merged_pot = this->merge_binary_and_unary(binary_to_merge, &unary_temp, Sum_or_MAP);
		merged_pot->Normalize_distribution();

		if (pwrapped == NULL)
			*diff_to_previous = FLT_MAX;
		else {
			eval_diff(diff_to_previous, this->I_Potential::Get_distr(this->pwrapped), this->I_Potential::Get_distr(merged_pot));
			delete this->pwrapped;
		}
		this->pwrapped = merged_pot;

	}
	
	Potential_Shape* Message_Unary::merge_binary_and_unary(Potential* binary_to_merge, Potential* unary, const bool& Sum_or_MAP) {

		Categoric_var* var_to_marginalize = I_Potential::Get_involved_var(binary_to_merge)->front(),
					 * var_to_remain = I_Potential::Get_involved_var(binary_to_merge)->back();
		size_t         pos_to_marginalize = 0, pos_to_remain = 1;
		if (var_to_remain == I_Potential::Get_involved_var(unary)->front()) {
			Categoric_var* C = var_to_remain;
			var_to_remain = var_to_marginalize;
			var_to_marginalize = C;

			size_t C2 = pos_to_remain;
			pos_to_remain = pos_to_marginalize;
			pos_to_marginalize = C2;
		}

		Potential_Shape* shape_temp = new Potential_Shape({ var_to_remain });
		auto shape_D = I_Potential::Get_distr(shape_temp);


		size_t k, K;

		list<I_Distribution_value*> unary_order;
		list<size_t*> val_to_search_temp;
		K = var_to_marginalize->size();
		for (k = 0; k < K; k++) {
			val_to_search_temp.push_back((size_t*)malloc(sizeof(size_t)));
			val_to_search_temp.back()[0] = k;
		}
		Find_Comb_in_distribution(&unary_order, val_to_search_temp, { var_to_marginalize }, unary);
		for (auto it = val_to_search_temp.begin(); it != val_to_search_temp.end(); it++)
			free(*it);

		list<I_Distribution_value*>::iterator it_unary_order;
		list<I_Distribution_value*> distr_temp;
		list<I_Distribution_value*>::iterator it_distr_temp;
		K = var_to_remain->size();
		size_t* comb;
		float result, temp1, temp2;
		for (k = 0; k < K; k++) {
			Find_Comb_in_distribution(&distr_temp, &k, { var_to_remain }, binary_to_merge);

			result = 0.f;
			for (it_distr_temp = distr_temp.begin(); it_distr_temp != distr_temp.end(); it_distr_temp++) {
				if (*it_distr_temp != NULL) {
					it_unary_order = unary_order.begin();
					advance(it_unary_order, (*it_distr_temp)->Get_indeces()[pos_to_marginalize]);

					if (*it_unary_order != NULL) {
						(*it_distr_temp)->Get_val(&temp1);
						(*it_unary_order)->Get_val(&temp2);

						temp1 *= temp2;
						if (Sum_or_MAP) result += temp1;
						else {
							if (temp1 > result) result = temp1;
						}
					}
				}
			}

			comb = (size_t*)malloc(sizeof(size_t));
			comb[0] = k;
			shape_D->push_back(new Distribution_value(comb));
			shape_D->back()->Set_val(result);
		}

		return shape_temp;

	}






	void I_Potential::Print_distribution(std::ostream& f, const bool& print_entire_domain) {

		list<I_Distribution_value*>* distribution_to_print;
		if (print_entire_domain) {
			list<size_t*> temp;
			Get_entire_domain(&temp, *this->Get_involved_var());
			list<I_Distribution_value*> ttemp;
			Find_Comb_in_distribution(&ttemp, temp, *this->Get_involved_var(), this);

			distribution_to_print = new list<I_Distribution_value*>();
			auto it_ttemp = ttemp.begin();
			float temp_val;
			for (auto it = temp.begin(); it != temp.end(); it++) {
				distribution_to_print->push_back(new Distribution_value(*it));

				if (*it_ttemp == NULL) distribution_to_print->back()->Set_val(0.f);
				else {
					distribution_to_print->back()->Get_val(&temp_val);
					distribution_to_print->back()->Set_val(temp_val);
				}

				it_ttemp++;
			}
		}
		else {
			distribution_to_print = this->Get_distr();
		}


		size_t var_num = this->Get_involved_var()->size(), k;
		float temp;
		auto itD_back = distribution_to_print->end();
		itD_back--;
		for (auto itD = distribution_to_print->begin(); itD != distribution_to_print->end(); itD++) {
			for (k = 0; k < var_num; k++)
				f << " " << (*itD)->Get_indeces()[k];
			(*itD)->Get_val(&temp);
			f << " " << temp;

			if (itD != itD_back)
				f << endl;
		}

		if (print_entire_domain) {
			for (auto it = distribution_to_print->begin(); it != distribution_to_print->end(); it++) 
				delete *it;
			delete distribution_to_print;
		}

	}

}