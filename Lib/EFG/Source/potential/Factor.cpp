#include <potential/Factor.h>
#include <distribution/FullMatchFinder.h>
#include <distribution/PartialMatchFinder.h>
#include <set>
#include <iostream>
#include <algorithm>
using namespace std;

namespace EFG::pot {
	
	Factor::Factor(const vector<CategoricVariable*>& var_involved) : IFactor<distr::DiscreteDistribution>(var_involved) { }

	Factor::Factor(const std::vector<CategoricVariable*>& var_involved, const std::string& file_to_read) : Factor(var_involved) {

		this->distribution.import(file_to_read);
		auto it = this->distribution.getIter();
		itr::forEach<distr::DiscreteDistribution::Iterator>(it, [this](distr::DiscreteDistribution::Iterator& itt) {
			if (itt->GetVal() < 0.f) {
				cout << "negative value ignored when importing from file a Factor \n";
				this->distribution.remove(itt->GetIndeces());
			}
		});

	}

	Factor::Factor(const std::vector<CategoricVariable*>& var_involved, const bool& correlated_or_not) : Factor(var_involved) {

		if (var_involved.size() <= 1) throw std::runtime_error("simply correlating potential must involve at least two variables");

		auto it = var_involved.begin();
		size_t Size = (*it)->size(); ++it;
		for (it; it != var_involved.end(); ++it) {
			if ((*it)->size() != Size) throw std::runtime_error("variables in a simply correlating potential must have all same sizes");
		}

		vector<size_t> comb;
		comb.reserve(this->distribution.GetVariables().size());
		if (correlated_or_not) {
			size_t kk;
			for (size_t k = 0; k < Size; ++k) {
				comb.clear();
				for (kk = 0; kk < var_involved.size(); ++kk) comb.push_back(k);
				this->distribution.add(comb, 1.f);
			}
		}
		else {
			this->SetOnes();
			size_t kk;
			distr::DiscreteDistribution::FullMatchFinder finder(this->distribution);
			for (size_t k = 0; k < Size; ++k) {
				comb.clear();
				for (kk = 0; kk < var_involved.size(); ++kk) comb.push_back(k);
				auto kkkk = finder(comb);
				finder(comb)->SetValRaw(0.f);
			}
		}

	}

	Factor::Factor(const Factor& to_copy, const std::vector<CategoricVariable*>& var_involved) : Factor(var_involved) {

		if (var_involved.size() != to_copy.GetDistribution().GetVariables().size()) throw std::runtime_error("invalid variables set");
		auto it = to_copy.GetDistribution().getIter();
		itr::forEach<distr::DiscreteDistribution::constIterator>(it, [this](distr::DiscreteDistribution::constIterator& itt) { 
			this->distribution.add(itt->GetIndeces(), itt->GetValRaw()); });
		
	}

	vector<CategoricVariable*> compute_merged_domain(const std::vector<const IPotential*>& potential_to_merge, const bool& merge_domain) {

		if (potential_to_merge.empty())  throw std::runtime_error("empty set of potentials to merge");

		if (!merge_domain) return potential_to_merge.front()->GetDistribution().GetVariables();

		vector<CategoricVariable*> vars;
		set<CategoricVariable*> passed;
		auto singlePotAdder = [&vars, &passed](const IPotential* p) {
			const std::vector<CategoricVariable*>& Vars = p->GetDistribution().GetVariables();
			size_t K = Vars.size();
			for (size_t k = 0; k < K; ++k) {
				if (passed.find(Vars[k]) == passed.end()) {
				// variable not already inside vars
					vars.push_back(Vars[k]);
					passed.emplace(Vars[k]);
				}
			}
		};
		std::for_each(potential_to_merge.begin(), potential_to_merge.end(), [&singlePotAdder](const IPotential* p) { singlePotAdder(p); });

		return vars;
	}
	Factor::Factor(const std::vector<const IPotential*>& potential_to_merge, const bool& use_sparse_format, const bool& merge_domain) :
		Factor(compute_merged_domain(potential_to_merge, merge_domain)) {

		list<distr::DiscreteDistribution::constFullMatchFinder> finders;
		auto doProduct = [this, &finders, &use_sparse_format](float& val, const std::size_t* comb) {
			auto itend = finders.end();
			const distr::DiscreteDistribution::Value* Val = nullptr;
			for (auto it = finders.begin(); it != itend; ++it) {
				Val = (*it)(comb);
				if (Val != nullptr) val *= Val->GetVal();
				else {
					val = 0.f;
					break;
				}
			}
			if (use_sparse_format) {
				if (val > 0.f) this->distribution.add(comb, val);
			}
			else this->distribution.add(comb, val);
		};

		auto vars = this->distribution.GetVariables();
		if (merge_domain) {
			std::for_each(potential_to_merge.begin(), potential_to_merge.end(), [&finders, &vars](const IPotential* p) { 
				finders.emplace_back(p->GetDistribution(), vars); });

			JointDomainIterator::forEach(vars, [&doProduct, &use_sparse_format](const vector<size_t>& comb) {
				float image = 1.f;
				doProduct(image, &comb[0]);
			});
		}
		else {
			size_t N_var = vars.size();
			auto it = potential_to_merge.begin();
			++it;
			std::for_each(it, potential_to_merge.end(), [&N_var, &finders, &vars](const IPotential* p) {
				if (p->GetDistribution().GetVariables().size() != N_var) throw std::runtime_error("potentials to merge must have all the same cardinality");
				finders.emplace_back(p->GetDistribution(), vars);
			});

			if (use_sparse_format) {
				auto itFirst = potential_to_merge.front()->GetDistribution().getIter();
				itr::forEach<distr::DiscreteDistribution::constIterator>(itFirst, [&doProduct](distr::DiscreteDistribution::constIterator& itt) {
					float image = itt->GetVal();
					doProduct(image, itt->GetIndeces());
				});
			}
			else {
				finders.emplace_front(potential_to_merge.front()->GetDistribution());
				JointDomainIterator::forEach(vars, [&doProduct, &use_sparse_format](const vector<size_t>& comb) {
					float image = 1.f;
					doProduct(image, &comb[0]);
				});
			}
		}

	};

	std::vector<CategoricVariable*> get_hidden_set(const std::vector<CategoricVariable*>& var_observed, const IPotential& pot_to_reduce) {

		auto vars = pot_to_reduce.GetDistribution().GetVariables();
		if (var_observed.size() >= vars.size()) throw std::runtime_error("at least one non observed variables must exist");

		list<CategoricVariable*> temp;
		for (auto it = vars.begin(); it != vars.end(); ++it) temp.push_back(*it);

		bool is_in_pot;
		size_t k, K = vars.size();
		for (auto ob = var_observed.begin(); ob != var_observed.end(); ++ob) {
			is_in_pot = false;
			for (k = 0; k < K; ++k) {
				if (vars[k] == *ob) {
					is_in_pot = true;
					break;
				}
			}
			if (!is_in_pot) throw std::runtime_error("invalid observations");
			temp.remove(*ob);
		}

		vector<CategoricVariable*> hidden;
		hidden.reserve(temp.size());
		std::for_each(temp.begin(), temp.end(), [&hidden](CategoricVariable* var) { hidden.push_back(var); });
		return hidden;

	}
	Factor::Factor(const std::vector<size_t>& val_observed, const std::vector<CategoricVariable*>& var_observed, const IPotential& pot_to_reduce) :
		Factor(get_hidden_set(var_observed, pot_to_reduce)) {

		if (val_observed.size() != var_observed.size()) throw std::runtime_error("number of observations must be equal to the number of observed variables");

		auto vars = pot_to_reduce.GetDistribution().GetVariables();
		size_t K = vars.size();

		distr::DiscreteDistribution::constPartialMatchFinder finder(pot_to_reduce.GetDistribution(), var_observed);
		auto remaining_val = finder(val_observed);

		auto remaining_var = this->distribution.GetVariables();
		size_t c, C = remaining_var.size();
		vector<size_t>		   remaining_var_pos;
		remaining_var_pos.reserve(C);
		size_t k;
		for (c = 0; c < C; ++c) {
			for (k = 0; k < K; ++k) {
				if (vars[k] == remaining_var[c]) {
					remaining_var_pos.push_back(k);
					break;
				}
			}
		}

		auto it_end = remaining_val.end();
		vector<size_t> comb;
		comb.reserve(C);
		for (auto it = remaining_val.begin(); it != it_end; ++it) {
			comb.clear();
			for (c = 0; c < C; ++c) comb.push_back((*it)->GetIndeces()[remaining_var_pos[c]]);
			this->distribution.add(comb, (*it)->GetVal());
		}

	}

	void Factor::SetOnes() {

		if (this->subject.isObserved()) throw std::runtime_error("cannot modify object while being observed");

		this->distribution.clear();

		JointDomainIterator::forEach(this->distribution.GetVariables() , [this](const vector<size_t>& com){ this->distribution.add(com, 1.f); });

	}

	void Factor::Normalize() {

		if (this->subject.isObserved()) throw std::runtime_error("cannot modify object while being observed");

		float Rescale = 0.f, temp = 0.f;
		{
			auto it = this->distribution.getIter();
			itr::forEach<distr::DiscreteDistribution::Iterator>(it, [&temp, &Rescale](distr::DiscreteDistribution::Iterator& itt) {
				temp = itt->GetVal();
				if (temp > Rescale) Rescale = temp;
			});
		}
		if (Rescale == 0.f) return;
		Rescale = 1.f / Rescale;

		auto it = this->distribution.getIter();
		itr::forEach<distr::DiscreteDistribution::Iterator>(it, [&Rescale](distr::DiscreteDistribution::Iterator& itt) { itt->SetValRaw(itt->GetValRaw() * Rescale); });

	}

	void Factor::Normalize2() {

		if (this->subject.isObserved()) throw std::runtime_error("cannot modify object while being observed");

		float Rescale = 0.f;
		{
			auto it = this->distribution.getIter();
			itr::forEach<distr::DiscreteDistribution::Iterator>(it, [&Rescale](distr::DiscreteDistribution::Iterator& itt) { Rescale += itt->GetVal(); });
		}

		if (Rescale == 0.f) {
			size_t S = this->distribution.size();
			if (S == 0) return;
			Rescale = 1.f / (float)S;
			auto it = this->distribution.getIter();
			itr::forEach<distr::DiscreteDistribution::Iterator>(it, [&Rescale](distr::DiscreteDistribution::Iterator& itt) { itt->SetValRaw(Rescale); });
		}
		else {
			Rescale = 1.f / Rescale;
			auto it = this->distribution.getIter();
			itr::forEach<distr::DiscreteDistribution::Iterator>(it, [&Rescale](distr::DiscreteDistribution::Iterator& itt) { itt->SetValRaw(itt->GetValRaw() * Rescale); });
		}

	}

}
