/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <Potential.h>
#include <ctime>
#include <float.h>
#include <cmath>
#include <iostream>
using namespace std;

namespace EFG {

	std::vector<float> I_Potential::Get_marginals() const {

		size_t Joint_Size = Categoric_var::Get_joint_domain_size(this->Get_Distribution().Get_Variables());

		float Z = 0.f;
		{
			auto it = this->Get_Distribution().get_iter();
			while (it.is_not_at_end()) {
				Z += it->Get_val();
				++it;
			}
		}
		if(Z < 1e-8) return vector<float>(Joint_Size , 1.f / (float)Joint_Size);

		Z = 1.f / Z;

		std::vector<float> prob_distr;
		prob_distr.reserve(Joint_Size);
		if(this->Get_Distribution().size() == Joint_Size){
			auto it = this->Get_Distribution().get_iter();
			while (it.is_not_at_end()) {
				prob_distr.push_back(it->Get_val() * Z);
				++it;
			}
		}
		else{
			Discrete_Distribution::const_Full_Match_finder finder(this->Get_Distribution());
			Domain_iterator domain(this->Get_Distribution().Get_Variables());
			while(domain.is_not_at_end()){
				auto val = finder(domain());
				if(val == nullptr) prob_distr.push_back(0.f);
				else prob_distr.push_back(val->Get_val() * Z);
				++domain;
			}
		}
		return prob_distr;

	}



	Potential_Shape::Potential_Shape(const vector<Categoric_var*>& var_involved) : I_Potential_Concrete<Discrete_Distribution>(var_involved) { }

	Potential_Shape::Potential_Shape(const std::vector<Categoric_var*>& var_involved, const std::string& file_to_read) : Potential_Shape(var_involved) { 
		
		this->distribution.import(file_to_read); 
		auto it = this->distribution.get_iter();
		while (it.is_not_at_end()) {
			if(it->Get_val() < 0.f){
#ifdef _DEBUG
					cout << "negative value ignored when importing from file a Potential_Shape \n";
#endif
				this->distribution.remove(it->Get_indeces());
			}
			++it;
		}
		
	}

	Potential_Shape::Potential_Shape(const std::vector<Categoric_var*>& var_involved, const bool& correlated_or_not) : Potential_Shape(var_involved) {

		if (var_involved.size() <= 1) throw std::runtime_error("simply correlating potential must involve at least two variables");
		
		auto it = var_involved.begin();
		size_t Size = (*it)->size(); ++it;
		for (it; it != var_involved.end(); ++it) {
			if ((*it)->size() != Size) throw std::runtime_error("variables in a simply correlating potential must have all same sizes");
		}

		vector<size_t> comb;
		comb.reserve(this->distribution.Get_Variables().size());
		if (correlated_or_not) {
			size_t kk;
			for (size_t k = 0; k < Size; ++k) {
				comb.clear();
				for (kk = 0; kk < var_involved.size(); ++kk) comb.push_back(k);
				this->distribution.add(comb, 1.f);
			}
		}
		else {
			this->Set_ones();
			size_t kk;
			Discrete_Distribution::Full_Match_finder finder(this->distribution);
			for (size_t k = 0; k < Size; ++k) {
				comb.clear();
				for (kk = 0; kk < var_involved.size(); ++kk) comb.push_back(k);
				finder(comb)->Set_val(0.f);
			}
		}

	}

	Potential_Shape::Potential_Shape(const Potential_Shape& to_copy, const std::vector<Categoric_var*>& var_involved) : Potential_Shape(var_involved) {

		if(var_involved.size() != to_copy.Get_Distribution().Get_Variables().size()) throw std::runtime_error("invalid variables set");
		auto it = to_copy.Get_Distribution().get_iter();
		while (it.is_not_at_end()) {
			this->distribution.add(it->Get_indeces() , it->Get_val());
			++it;
		}

	}

	vector<Categoric_var*> compute_merged_domain(const std::vector<const I_Potential*>& potential_to_merge, const bool& merge_domain){

		if (potential_to_merge.empty())  throw std::runtime_error("empty set of potentials to merge");

		if(!merge_domain) return potential_to_merge.front()->Get_Distribution().Get_Variables();

		vector<Categoric_var*> vars;
		vars.reserve(6);
		size_t v,V;
		size_t p,P;
		bool add;
		for(auto it= potential_to_merge.begin(); it!=potential_to_merge.end(); ++it){
			const vector<Categoric_var*>& pot_vars = (*it)->Get_Distribution().Get_Variables();
			P = pot_vars.size();
			for(p=0; p<P; ++p){
				V = vars.size();
				add = true;
				for(v=0; v<V; ++v){
					if(vars[v] == pot_vars[p]){
						add = false;
						break;
					}
				}
				if(add) vars.push_back(pot_vars[p]);
			}
		}
		return vars;
	}
	Potential_Shape::Potential_Shape(const std::vector<const I_Potential*>& potential_to_merge, const bool& use_sparse_format, const bool& merge_domain) : 
	Potential_Shape(compute_merged_domain(potential_to_merge , merge_domain)) {

		list<Discrete_Distribution::const_Full_Match_finder> finders;
		list<Discrete_Distribution::const_Full_Match_finder>::iterator it, it_end;
		float image ;
		if(merge_domain){
			auto vars = this->distribution.Get_Variables();
			for(size_t k=0; k<potential_to_merge.size(); ++k) finders.emplace_back(potential_to_merge[k]->Get_Distribution() , vars);
			Domain_iterator domain_combination(vars);
			bool add;
			it_end = finders.end();
			while(domain_combination.is_not_at_end()){
				image = 1.f;
				for(it = finders.begin(); it!=it_end; ++it){
					auto val = (*it)(domain_combination());
					if(val != nullptr) image *= val->Get_val();
					else {
						image = 0.f;
						break;
					}
				}
				add = true;
				if(use_sparse_format && (image == 0.f)) add = false;
				if(add) this->distribution.add(domain_combination(), image);
				++domain_combination;
			}
		}
		else{

			auto vars = potential_to_merge.front()->Get_Distribution().Get_Variables();
			size_t N_var = vars.size();
			for(size_t k=1; k<potential_to_merge.size(); ++k){
				if(potential_to_merge[k]->Get_Distribution().Get_Variables().size() != N_var) throw std::runtime_error("potentials to merge must have all the same cardinality");
				finders.emplace_back(potential_to_merge[k]->Get_Distribution(), vars);
			}

			it_end = finders.end();
			if(use_sparse_format){
				auto it2 = potential_to_merge.front()->Get_Distribution().get_iter();
				while(it2.is_not_at_end()){
					image = it2->Get_val();
					for(it = finders.begin(); it!=it_end; ++it){
						auto val = (*it)(it2->Get_indeces());
						if(val != nullptr) image *= val->Get_val();
						else {
							image = 0.f;
							break;
						}
					}
					if(image != 0.f) this->distribution.add(it2->Get_indeces(), image);
					++it2;
				}
			}
			else{
				finders.emplace_front(potential_to_merge.front()->Get_Distribution());
				Domain_iterator domain_combination(potential_to_merge.front()->Get_Distribution().Get_Variables()); 
				while(domain_combination.is_not_at_end()){
					image = 1.f;
					for(it = finders.begin(); it!=it_end; ++it){
						auto val = (*it)(domain_combination());
						if(val != nullptr) image *= val->Get_val();
						else {
							image = 0.f;
							break;
						}
					}
					this->distribution.add(domain_combination(), image);
					++domain_combination;
				}
			}
		}

	};

	std::vector<Categoric_var*> get_hidden_set(const std::vector<Categoric_var*>& var_observed, const I_Potential& pot_to_reduce){

		auto vars = pot_to_reduce.Get_Distribution().Get_Variables();
		if (var_observed.size() >= vars.size() ) throw std::runtime_error("at least one non observed variables must exist");

		list<Categoric_var*> temp;
		for(auto it=vars.begin(); it!=vars.end(); ++it) temp.push_back(*it);

		bool is_in_pot;
		size_t k, K = vars.size();
		for(auto ob = var_observed.begin(); ob!=var_observed.end(); ++ob){
			is_in_pot = false;
			for(k=0; k<K; ++k) {
				if(vars[k] == *ob){
					is_in_pot =  true;
					break;
				}
			}
			if(!is_in_pot) throw std::runtime_error("invalid observations");
			temp.remove(*ob);
		}

		vector<Categoric_var*> hidden;
		hidden.reserve(temp.size());
		for(auto it = temp.begin(); it!=temp.end(); ++it) hidden.push_back(*it);
		return hidden;

	}
	Potential_Shape::Potential_Shape(const std::vector<size_t>& val_observed, const std::vector<Categoric_var*>& var_observed, const I_Potential& pot_to_reduce) :
	Potential_Shape(get_hidden_set(var_observed, pot_to_reduce)) {

		if (val_observed.size() != var_observed.size()) throw std::runtime_error("number of observations must be equal to the number of observed variables");
		
		auto vars = pot_to_reduce.Get_Distribution().Get_Variables();
		size_t K = vars.size();

		Discrete_Distribution::const_Partial_Match_finder finder(pot_to_reduce.Get_Distribution(), var_observed);
		auto remaining_val = finder(val_observed);

		auto remaining_var = this->distribution.Get_Variables();
		size_t c, C = remaining_var.size();
		vector<size_t>		   remaining_var_pos;
		remaining_var_pos.reserve(C);
		size_t k;
		for(c = 0; c<C; ++c){
			for(k=0; k<K; ++k){
				if(vars[k] == remaining_var[c]) {
					remaining_var_pos.push_back(k);
					break;
				}
			}
		}

		auto it_end = remaining_val.end();
		vector<size_t> comb;
		comb.reserve(C);
		for(auto it = remaining_val.begin(); it!=it_end; ++it){
			comb.clear();
			for(c=0; c<C; ++c) comb.push_back((*it)->Get_indeces()[remaining_var_pos[c]]);
			this->distribution.add(comb , (*it)->Get_val());
		}

	}



	void Potential_Shape::Set_ones() {

		if(this->subject.is_observed()) throw std::runtime_error("cannot modify object while being observed");

		this->distribution.clear();

		Domain_iterator domain(this->distribution.Get_Variables());
		while(domain.is_not_at_end()){
			this->distribution.add(domain() , 1.f);
			++domain;
		}

	}

	void Potential_Shape::Normalize_distribution() {

		if (this->subject.is_observed()) throw std::runtime_error("cannot modify object while being observed");

		float Rescale = 0.f, temp = 0.f;
		{
			auto it = this->distribution.get_iter();
			while(it.is_not_at_end()){
				temp = it->Get_val();
				if(temp > Rescale) Rescale = temp;
				++it;
			}	
		}
		if (Rescale == 0.f) return;
		Rescale = 1.f / Rescale;
		
		auto it = this->distribution.get_iter();
		while(it.is_not_at_end()){
			it->Set_val(it->Get_val() * Rescale);
			++it;
		}	

	}

	void Potential_Shape::Normalize_distribution2() {

		if (this->subject.is_observed()) throw std::runtime_error("cannot modify object while being observed");

		float Rescale = 0.f;
		{
			auto it = this->distribution.get_iter();
			while (it.is_not_at_end()) {
				Rescale += it->Get_val();
				++it;
			}
		}

		if (Rescale == 0.f) {
			size_t S = this->distribution.size();
			if (S == 0) return;
			Rescale = 1.f / (float)S;
			auto it = this->distribution.get_iter();
			while (it.is_not_at_end()) {
				it->Set_val(Rescale);
				++it;
			}
		}
		else {
			Rescale = 1.f / Rescale;

			auto it = this->distribution.get_iter();
			while (it.is_not_at_end()) {
				it->Set_val(it->Get_val() * Rescale);
				++it;
			}
		}

	}



	Potential_Exp_Shape::Potential_Exp_Shape(const Potential_Shape& shape, const float& w) : I_Potential_Concrete<Discrete_exp_Distribution>(shape.Get_Distribution().Get_Variables()) { 

		this->distribution.set_weight(w);

		if(Categoric_var::Get_joint_domain_size(this->distribution.Get_Variables()) == shape.Get_Distribution().size()){
			auto it = shape.Get_Distribution().get_iter();
			while(it.is_not_at_end()){
				this->distribution.add(it->Get_indeces() , it->Get_val());
				++it;
			}
		}
		else{
			Discrete_Distribution::const_Full_Match_finder finder(shape.Get_Distribution());
			Domain_iterator domain(this->distribution.Get_Variables());
			while(domain.is_not_at_end()){
				auto val = finder(domain());
				if(val == nullptr) 	this->distribution.add(domain() , 0.f);
				else 				this->distribution.add(val->Get_indeces() , val->Get_val());
				++domain;
			}
		}

	};

	Potential_Exp_Shape::Potential_Exp_Shape(const std::vector<Categoric_var*>& var_involved, const std::string& file_to_read, const float& w) :  
	Potential_Exp_Shape(Potential_Shape(var_involved, file_to_read), w) { };

	Potential_Exp_Shape::Potential_Exp_Shape(const Potential_Exp_Shape& to_copy, const std::vector<Categoric_var*>& var_involved) : I_Potential_Concrete<Discrete_exp_Distribution>(var_involved) { 

		if(var_involved.size() != to_copy.distribution.Get_Variables().size()) throw std::runtime_error("invalid variable set");
		
		this->distribution.set_weight(to_copy.distribution.get_weight());

		auto it = to_copy.distribution.get_iter();
		while(it.is_not_at_end()){
			this->distribution.add(it->Get_indeces() , it->Get_val_raw());
			++it;
		}
	};

	Potential_Exp_Shape::weigth_modifier::weigth_modifier(Potential_Exp_Shape& involved_pot) : Subject::Observer(&involved_pot.subject_w_hndlr), pot(involved_pot) {};

}
