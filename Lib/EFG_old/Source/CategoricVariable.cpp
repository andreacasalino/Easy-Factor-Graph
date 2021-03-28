#include <CategoricVariable.h>
#include <Error.h>
#include <algorithm>
#include <set>
using namespace std;

namespace EFG {

    CategoricVariable::CategoricVariable(const std::size_t& size, const std::string& name) :Size(size), Name(name) {

        if (name.size() == 0)  throw Error("CategoricVariable", "empty name forbidden");

        if (size == 0) throw Error("CategoricVariable", "null size forbidden");

    }

	CategoricVariable::CategoricVariable(const CategoricVariable& o) : Size(o.Size), Name(o.Name) { } //MultiObservable reconstructed

    bool CategoricVariable::AreAllVarsDifferent(const std::vector<CategoricVariable*>& variables){

        if(variables.empty()) throw Error("CategoricVariable", "empty set cannot be check");

        set<CategoricVariable*> passed;
		for (auto it = variables.begin(); it != variables.end(); ++it) {
			if (passed.find(*it) == passed.end()) passed.emplace(*it);
			else return false;
		}
		return true;

    }

    std::size_t CategoricVariable::GetJointDomainSize(const std::vector<CategoricVariable*>& vars){

        std::size_t S = 1;
		std::for_each(vars.begin(), vars.end(), [&S](CategoricVariable* var) { S *= var->size(); });
        return S;

    }



	itr::Iterator& JointDomainIterator::operator++() {

		if (!this->isNotAtEnd()) throw Error("JointDomainIterator", "not incrementable");

		size_t k = this->Comb.size() - 1;
		while (true) {
			++this->Comb[k];
			if (this->Comb[k] == this->Sizes[k]) {
				if (k == 0) {
					this->isActive = false;
					break;
				}
				else {
					this->Comb[k] = 0;
					--k;
				}
			}
			else break;
		}

		return *this;

	};

	JointDomainIterator::JointDomainIterator(const std::vector<CategoricVariable*>& vars) {

		if (!CategoricVariable::AreAllVarsDifferent(vars)) throw Error("JointDomainIterator", "found repeated variables in domain");

		this->Sizes.reserve(vars.size());
		this->Comb.reserve(vars.size());
		for (size_t k = 0; k < vars.size(); ++k) {
			this->Sizes.push_back(vars[k]->size());
			this->Comb.push_back(0);
		}

	}

	void JointDomainIterator::forEach(const std::vector<CategoricVariable*>& vars, const std::function<void(const std::vector<size_t>&)>& action) {
		JointDomainIterator temp(vars);
		itr::forEach<JointDomainIterator>(temp, [&action](JointDomainIterator& it) { action(it()); });
	}

	const std::vector<size_t>& JointDomainIterator::operator()() const { 
		if (!this->isNotAtEnd()) throw Error("JointDomainIterator", "not dereferenciable"); 
		return this->Comb; 
	};

}
