#ifndef _UNIVOCAL_MAP_H_
#define _UNIVOCAL_MAP_H_

#include <unordered_map>
#include <functional>

namespace EFG::ump {

	/* Key_Generator must be a unary function object with the following operator defined: Key operator()(T&)
	*/
	template<typename Key, typename T, typename Key_Generator, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
	class univocal_map{
	public:

	~univocal_map() { for(auto it=this->map.begin(); it!=this->map.end(); ++it) delete it->second; };

	univocal_map(const Key_Generator& gen = Key_Generator(), const Hash& hsh = Hash(), const Pred& pred = Pred()) : 
	generator(gen), map(0, hsh, pred) {};

	inline const Key_Generator&                      get_generator() const { return this->generator; };

	template<typename ... Args>
	void emplace(Args ... args){

		T* temp = new T(args ...);
		this->map.insert(std::make_pair(this->generator(*temp),temp));

	};

	/* Builder must be a predicate taking args as input and returning an instance of T allocated in the heap
	*/
	template<typename Builder , typename ... Args>
	void emplace_with_builder(const Builder& bld, Args ... args){

		T* temp = bld(args ...);
		this->map.insert(std::make_pair(this->generator(*temp),temp));

	};

	inline const std::unordered_map<Key, T*, Hash, Pred>*   get_map() const { return &this->map; };

	inline void clear() { for(auto it=this->map.begin(); it!=this->map.end(); ++it) delete it->second; this->map.clear(); };

	protected:
	// data
		Key_Generator                             generator;
		std::unordered_map<Key, T*, Hash, Pred>   map;
	};

}

#endif