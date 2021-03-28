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
#include <distribution/Group.h>
#include <ErrorRaiser.h>

namespace EFG::distr {

    class DiscreteDistribution : public Group {
    public:
        class Value;

        template<typename Array>
        Value*                   add(const Array& comb, const float& val);

        template<typename Array>
        void                    remove(const Array& comb);

        inline void             clear() { this->Map.clear(); };

        void                    import(const std::string& file_to_read);

        ~DiscreteDistribution();

        DiscreteDistribution(const std::vector<CategoricVariable*>& vars);

        DiscreteDistribution(DiscreteDistribution&& o);
        void operator=(DiscreteDistribution&& o);

        inline std::size_t size() const { return this->Map.size(); };

        class       FullMatchFinder;
        class  constFullMatchFinder;

        class       PartialMatchFinder; 
        class  constPartialMatchFinder;  

        class       Iterator;
        inline      Iterator    getIter();
        class  constIterator;       
        inline constIterator    getIter() const;
    protected:
        inline virtual float evalImage(const float& valRaw) const { return valRaw; };
    private:
        class Ifinder;
        class IFullfinder;
        class IPartialfinder;

        struct Key{        
            const std::size_t*                                          combination;
            const std::vector<size_t>*                                  var_order;
        };   
        
        struct comparator { 
            comparator(const std::size_t& N) : N_variables(N) {};
            std::size_t                      N_variables;
            bool operator()(const Key& a, const Key& b) const; 
        };

        typedef std::map<Key, Value*, comparator> Distribution_map;
    // data
        Distribution_map                  Map;
        sbj::MultiObservable              subjFinders;
    };



    class DiscreteDistribution::Value{
        friend class DiscreteDistribution;
    public:
        ~Value();

        inline float		  GetVal() const { return this->source->evalImage(this->valRaw); };
        inline const float&   GetValRaw() const { return this->valRaw; };
        inline const std::size_t*  GetIndeces() const { return this->combination; };

        inline void           SetValRaw(const float& val) { this->valRaw = val; };
    private:
        Value(DiscreteDistribution* distr, std::size_t* comb, const float& val) : source(distr), combination(comb), valRaw(val) {};
    // data
        const DiscreteDistribution*       source;
        std::size_t*                      combination;
        float                             valRaw;
    };



    template<typename Array>
    DiscreteDistribution::Value* DiscreteDistribution::add(const Array& comb, const float& val){

        const std::vector<CategoricVariable*>& Vars = this->GetVariables();
        std::size_t K = Vars.size();
        std::size_t* clone = (size_t*)malloc(sizeof(size_t)*K);
        for(size_t k=0; k<K;++k){
            clone[k] = comb[k];
            if (clone[k] >= Vars[k]->size()) {
                free(clone);
                raiseError("distr::DiscreteDistribution", "combination out of bounds");
            } 
        }

        Key key = {clone, nullptr};
        if(this->Map.find(key) != this->Map.end()) {
            free(clone);
            raiseError("distr::DiscreteDistribution", "combination already added");
        }

        Value* to_add = new Value(this, clone, val);
        auto temp = this->Map.emplace(key , to_add);
        return temp.first->second;

    };

    template<typename Array>
    void DiscreteDistribution::remove(const Array& comb){

        Key key = {&comb[0], nullptr};
        auto it = this->Map.find(key);
        if(it == this->Map.end()) raiseError("distr::DiscreteDistribution", "inexistent combination");
        this->Map.erase(it);

    }



    class DiscreteDistribution::Ifinder {
    protected:
        const DiscreteDistribution*                   source;
        sbj::Subject::Observer                        sourceObsv;
    public:
        Ifinder(const DiscreteDistribution& distr) : source(&distr), sourceObsv(distr.subjFinders) {};

        Ifinder(const Ifinder& ) = delete;
        void operator=(const Ifinder& ) = delete;
    };




    class DiscreteDistribution::Iterator : public EFG::itr::TypedIterator<Distribution_map::iterator>{
        friend class DiscreteDistribution;
        Iterator(DiscreteDistribution& toIterate);
    public:
        inline Value*    operator->() { return this->cursor->second; };
    };
    DiscreteDistribution::Iterator DiscreteDistribution::getIter() { return Iterator(*this); };
    
    class DiscreteDistribution::constIterator : public EFG::itr::TypedIterator<Distribution_map::const_iterator> {
        friend class DiscreteDistribution;
        constIterator(const DiscreteDistribution& toIterate);
    public:
        inline const Value*    operator->() const { return this->cursor->second; };
    };
    DiscreteDistribution::constIterator DiscreteDistribution::getIter() const { return constIterator(*this); };

}

#endif
