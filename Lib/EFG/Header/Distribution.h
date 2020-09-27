/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_DISTRIBUTION_H__
#define __EFG_DISTRIBUTION_H__

#include <map>
#include <utility>
#include <math.h>
#include <Categoric_Variable.h>


namespace EFG {

    class I_Discrete{
    public:
        inline const std::vector<Categoric_var*>&     Get_Variables() const { return this->Variables; };
        		
		/** \brief Use this method for replacing the set of variables in the domain of this distribution.
		* Variables in new_var must be equal in number to the original set of variables and
		* must have the same sizes.
        *
		* @param[in] new_var variables to consider for the substitution
		*/
		void Substitute_variables(const std::vector<Categoric_var*>& new_var);

        virtual ~I_Discrete() {};
    protected:
        I_Discrete(const std::vector<Categoric_var*>& vars);
        I_Discrete(const I_Discrete& ) = delete;
        void operator=(const I_Discrete& ) = delete;

        template<typename iter>
        class I_iterator{
        public:
            inline bool  is_not_at_end() const { return (this->cursor != this->cursor_end); };
        protected:
            iter   cursor;
            iter   cursor_end;        
        };
    private:
    // data
        std::vector<Categoric_var*>     Variables;
        std::vector<Subject::Observer>  Variables_observers;
    };



    class Discrete_Domain : public I_Discrete {
    public:
        Discrete_Domain(const std::vector<Categoric_var*>& vars) : I_Discrete(vars) {};
        ~Discrete_Domain() { for(auto it=this->domain.begin(); it!=this->domain.end(); ++it) free(*it); };

        inline size_t size() const { return this->domain.size(); };

        template<typename Array>
        void                    add(const Array& comb);

        class const_iterator;       
        inline const_iterator    get_iter() const;
    protected:
    // data
        std::list<size_t*>      domain;
    };

    template<typename Array>
    void  Discrete_Domain::add(const Array& comb){

        const std::vector<Categoric_var>& vars = this->Get_Variables();
        size_t K = vars.size();
        size_t* clone = (size_t*)malloc(sizeof(size_t)*K);
        for(size_t k=0; k<K;++k){
            clone[k] = comb[k];
            if (clone[k] >= vars[k].size()) {
                free(clone);
                throw std::runtime_error("combination out of bounds");
            }
        }
        this->domain.push_back(clone);

    };

    class Discrete_Domain::const_iterator : public I_iterator<std::list<size_t*>::const_iterator> {
        friend class Discrete_Domain;
        const_iterator(const Discrete_Domain& to_iterate) { cursor = to_iterate.domain.begin(); cursor_end = to_iterate.domain.end(); };
    public:
        inline const size_t*    operator*() { return *this->cursor; };
        inline const_iterator& operator++() { ++this->cursor; return *this; };
        inline const_iterator& operator--() { --this->cursor; return *this; };
    };
    Discrete_Domain::const_iterator Discrete_Domain::get_iter() const { return const_iterator(*this); };



    class Discrete_Distribution : public I_Discrete{
    public:
        class Value;

        template<typename Array>
        Value*                   add(const Array& comb, const float& val);

        template<typename Array>
        void                    remove(const Array& comb);

        inline void             clear() { this->Map.clear(); };

        void                    import(const std::string& file_to_read);

        Discrete_Distribution(const std::vector<Categoric_var*>& vars);
        ~Discrete_Distribution();

        inline size_t size() const { return this->Map.size(); };

        class       Full_Match_finder;
        class const_Full_Match_finder;

        class       Partial_Match_finder; 
        class const_Partial_Match_finder;  

        class       iterator;
        inline iterator          get_iter();
        class const_iterator;       
        inline const_iterator    get_iter() const;
    protected:      
        inline virtual float                      eval_image(const float& val) const { return val; };
    private:
        template<typename Distribution>
        class I_finder;

        template<typename Distribution>
        class I_Full_Match_finder;

        template<typename Distribution>
        class I_Partial_Match_finder;

        struct Key{        
            const size_t*                                               combination;
            const std::vector<size_t>*                                  var_order;
        };   
        
        struct comparator { 
            comparator(const size_t& N) : N_variables(N) {};
            size_t                      N_variables;
            bool operator()(const Key& a, const Key& b) const; 
        };

        typedef std::map<Key, Value*, comparator> Distribution_map;

    // data
        Distribution_map                Map;
    };



    class Discrete_Distribution::Value{
        friend class Discrete_Distribution;
    public:
        ~Value();

        inline float		  Get_val() const { return this->Distribution_source->eval_image(this->value_4_image_computation); };
        inline const float&   Get_val_raw() const { return this->value_4_image_computation; };
        inline const size_t*  Get_indeces() const { return this->combination; };

        inline void           Set_val(const float& val) { this->value_4_image_computation = val; };
    private:
        Value(Discrete_Distribution* distr, size_t* comb, const float& val) : Distribution_source(distr), combination(comb), value_4_image_computation(val) {};
    // data
        const Discrete_Distribution*       Distribution_source;
        size_t*                            combination;
        float                              value_4_image_computation;
    };

    template<typename Array>
    Discrete_Distribution::Value*  Discrete_Distribution::add(const Array& comb, const float& val){

        const std::vector<Categoric_var*>& Vars = this->Get_Variables();
        size_t K = Vars.size();
        size_t* clone = (size_t*)malloc(sizeof(size_t)*K);
        for(size_t k=0; k<K;++k){
            clone[k] = comb[k];
            if (clone[k] >= Vars[k]->size()) {
                free(clone);
                throw std::runtime_error("combination out of bounds");
            } 
        }

        Key key = {clone, nullptr};
        if(this->Map.find(key) != this->Map.end()) {
            free(clone);
            throw std::runtime_error("combination already added");
        }

        Value* to_add = new Value(this, clone, val);
        auto temp = this->Map.emplace(key , to_add);
        return temp.first->second;

    };

    template<typename Array>
    void Discrete_Distribution::remove(const Array& comb){

        Key key = {&comb[0], nullptr};
        auto it = this->Map.find(key);
        if(it == this->Map.end()) throw std::runtime_error("inexistent combination");
        this->Map.erase(it);

    }



    template<typename Distribution>
    class Discrete_Distribution::I_finder{
    protected:
        Distribution*                   Distribution_source;
    public:
        I_finder(Distribution& distr) : Distribution_source(&distr) {};

        I_finder(const I_finder& ) = delete;
        void operator=(const I_finder& ) = delete;
    };



    template<typename Distribution>
    class Discrete_Distribution::I_Full_Match_finder : public Discrete_Distribution::I_finder<Distribution>{
    protected:
        I_Full_Match_finder(Distribution& distrib) : I_finder<Distribution>(distrib), var_order(nullptr) { };
        I_Full_Match_finder(Distribution& distrib, const std::vector<Categoric_var*>& vars) : I_Full_Match_finder<Distribution>(distrib) {

            auto distr_vars = distrib.Get_Variables();
            if(vars.size() < distr_vars.size()) throw std::runtime_error("number of vars must be higher or equal to the domain cardinality");
            if(!Categoric_var::check_all_vars_are_different(vars)) throw std::runtime_error("repeated variable in set");

            this->var_order = new std::vector<size_t>();
            this->var_order->reserve(distr_vars.size());
            size_t K2 = vars.size();
            for(size_t k=0; k<distr_vars.size(); ++k){
                for(size_t k2 = 0; k2<K2; ++k2){
                    if(distr_vars[k] == vars[k2]) {
                        this->var_order->push_back(k2);
                        break;
                    }
                }
            }
            if(this->var_order->size() != distrib.Get_Variables().size()) throw std::runtime_error("some variables in the domain of the distributon were not found in vars");

        };
        ~I_Full_Match_finder() { delete this->var_order; };

        template<typename Array, typename Val>
        Val*                              __find(const Array& combination) const{

            auto it = this->Distribution_source->Map.find(Discrete_Distribution::Key{&combination[0] , this->var_order});
            if(it == this->Distribution_source->Map.end()) return nullptr;
            return it->second;

        };
    private:
    // data
        std::vector<size_t>*                                 var_order;
    };

    class Discrete_Distribution::Full_Match_finder : public Discrete_Distribution::I_Full_Match_finder<Discrete_Distribution> {
    public:
        Full_Match_finder(Discrete_Distribution& distrib) : I_Full_Match_finder<Discrete_Distribution>(distrib) {};
        Full_Match_finder(Discrete_Distribution& distrib, const std::vector<Categoric_var*>& vars) : I_Full_Match_finder<Discrete_Distribution>(distrib, vars) {};

        template<typename Array>
        inline Value*                     operator()(const Array& combination) {  return this->__find<Array, Value>(combination); };
    };

    class Discrete_Distribution::const_Full_Match_finder : public Discrete_Distribution::I_Full_Match_finder<const Discrete_Distribution> {
    public:
        const_Full_Match_finder(const Discrete_Distribution& distrib) : I_Full_Match_finder<const Discrete_Distribution>(distrib) {};
        const_Full_Match_finder(const Discrete_Distribution& distrib, const std::vector<Categoric_var*>& vars) : I_Full_Match_finder<const Discrete_Distribution>(distrib, vars) {};

        template<typename Array>
        inline const Value*               operator()(const Array& combination)  const {  return this->__find<Array, const Value>(combination); };
    };



    template<typename Distribution>
    class Discrete_Distribution::I_Partial_Match_finder : public Discrete_Distribution::I_finder<Distribution> {
    protected:
        I_Partial_Match_finder(Distribution& distrib, const std::vector<Categoric_var*>& vars) : I_finder<Distribution>(distrib){

            auto distr_vars = distrib.Get_Variables();
            if(vars.size() >= distr_vars.size()) throw std::runtime_error("number of vars must be lower than the domain cardinality");
            if(!Categoric_var::check_all_vars_are_different(vars)) throw std::runtime_error("repeated variable in set");

            this->var_order.reserve(vars.size());
            size_t K2 = distr_vars.size();
            for(size_t k=0; k<vars.size(); ++k){
                for(size_t k2 = 0; k2<K2; ++k2){
                    if(vars[k] == distr_vars[k2]){
                        this->var_order.push_back(k2);
                        break;
                    }
                }
            }
            if(this->var_order.size() != vars.size()) throw std::runtime_error("some variables in vars were not found in this domain");

        };

        template<typename Array, typename Val>
        std::list<Val*>                   __find(const Array& combination) const{

            std::list<Val*> match;
            bool flag;
            for(auto it=this->Distribution_source->Map.begin(); it!=this->Distribution_source->Map.end(); ++it){
                flag = true;
                for(size_t k=0; k<this->var_order.size(); ++k){
                    if(combination[k] != it->second->Get_indeces()[this->var_order[k]]) {
                        flag = false;
                        break;
                    }
                }
                if(flag) match.push_back(it->second);
            }
            return match;

        };
    private:
    // data
        std::vector<size_t>                                 var_order;
    };

    class Discrete_Distribution::Partial_Match_finder : public Discrete_Distribution::I_Partial_Match_finder<Discrete_Distribution> {
    public:
        Partial_Match_finder(Discrete_Distribution& distrib, const std::vector<Categoric_var*>& vars) : I_Partial_Match_finder<Discrete_Distribution>(distrib, vars) {};

        template<typename Array>
        inline std::list<Value*>                      operator()(const Array& combination) {  return this->__find<Array, Value>(combination); };
    };

    class Discrete_Distribution::const_Partial_Match_finder : public Discrete_Distribution::I_Partial_Match_finder<const Discrete_Distribution> {
    public:
        const_Partial_Match_finder(const Discrete_Distribution& distrib, const std::vector<Categoric_var*>& vars) : I_Partial_Match_finder<const Discrete_Distribution>(distrib, vars) {};

        template<typename Array>
        inline std::list<const Value*>              operator()(const Array& combination)  const {  return this->__find<Array, const Value>(combination); };
    };




    class Discrete_Distribution::iterator : public I_iterator<Distribution_map::iterator> {
        friend class Discrete_Distribution;
        iterator(Discrete_Distribution& to_iterate) { cursor = to_iterate.Map.begin(); cursor_end = to_iterate.Map.end(); };
    public:
        inline Value*    operator->() { return this->cursor->second; };
        inline iterator& operator++() { ++this->cursor; return *this; };
        inline iterator& operator--() { --this->cursor; return *this; };
    };
    Discrete_Distribution::iterator Discrete_Distribution::get_iter() { return iterator(*this); };
    
    class Discrete_Distribution::const_iterator : public I_iterator<Distribution_map::const_iterator> {
        friend class Discrete_Distribution;
        const_iterator(const Discrete_Distribution& to_iterate) { cursor = to_iterate.Map.begin(); cursor_end = to_iterate.Map.end(); };
    public:
        inline const Value*    operator->() const { return this->cursor->second; };
        inline const_iterator& operator++() { ++this->cursor; return *this; };
        inline const_iterator& operator--() { --this->cursor; return *this; };
    };
    Discrete_Distribution::const_iterator Discrete_Distribution::get_iter() const { return const_iterator(*this); };



    class Discrete_exp_Distribution : public Discrete_Distribution {
    public:
        Discrete_exp_Distribution(const std::vector<Categoric_var*>& vars, const float w = 1.f) : Discrete_Distribution(vars) {};

        inline const float&                       get_weight() const { return this->weight; };
        inline void                               set_weight(const float& w) { this->weight = w; };
    private:
        inline float                      eval_image(const float& val) const override { return expf(val * this->weight); };

        float                           weight;
    };

}

#endif
