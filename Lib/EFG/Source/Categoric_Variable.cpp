#include <Categoric_Variable.h>
using namespace std;

namespace EFG {

    Categoric_var::Categoric_var(const size_t& size, const std::string& name) :Size(size), Name(name) {

        if (name.size() == 0)  throw std::runtime_error("empty name for Categoric variable");

        if (size == 0) throw std::runtime_error("null size of categorical variable");

    }

    bool Categoric_var::check_all_vars_are_different(const std::vector<Categoric_var*>& variables){

        if(variables.empty()) throw std::runtime_error("empty set of variables cannot be check");

        list<Categoric_var*> passed;
        list<Categoric_var*>::iterator it2;
        for(auto it=variables.begin(); it!=variables.end(); ++it){
            for(it2 = passed.begin(); it2 != passed.end(); ++it2){
                if((*it2)->Get_name().compare((*it)->Get_name()) == 0) return false;
            }
			passed.push_back(*it);
        }
        return true;

    }

    size_t Categoric_var::Get_joint_domain_size(const std::vector<Categoric_var*>& vars){

        size_t S = 1;
        size_t K =vars.size();
        for(size_t k=0; k<K; ++k) S *= vars[k]->size();
        return S;

    }



	Domain_iterator& Domain_iterator::operator++(){
		
		if(!this->flag) throw std::runtime_error("iterator not incrementable");

		size_t k = this->Comb.size() - 1;
		while(true){
			++this->Comb[k];
			if(this->Comb[k] == this->Sizes[k]){
				if(k == 0) {
					this->flag = false;
					break;
				}
				else{
					this->Comb[k] = 0;
					--k;
				}
			}
			else break;
		}

		return *this;

	};

	Domain_iterator::Domain_iterator(const std::vector<Categoric_var*>& vars) {

		if(!Categoric_var::check_all_vars_are_different(vars)) throw std::runtime_error("found repeated variables in domain");
		this->init(vars);

	}

	void Domain_iterator::init(const std::vector<Categoric_var*>& vars){

		this->flag = true;

		this->Sizes.reserve(vars.size());
		this->Comb.reserve(vars.size());
		for(size_t k=0; k<vars.size(); ++k){
			this->Sizes.push_back(vars[k]->size());
			this->Comb.push_back(0);
		}

	}


}
