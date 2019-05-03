//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_POTENTIAL_H__
#define __CRF_POTENTIAL_H__

#include <list>
#include <fstream>


namespace Segugio {

	class Categoric_var {
	public:
	//domain is assumed to be {0,1,2,3,...,size}
		Categoric_var(const size_t& size, const std::string& name); //name cannot be empty string ""
		Categoric_var(const Categoric_var& to_copy) { abort(); };
		virtual ~Categoric_var() {};

		const size_t& size() { return this->Size; };
		const std::string& Get_name() { return this->Name; };
	protected:
	// data
		size_t		Size;
		std::string Name;
	};


	class Categoric_domain : public Categoric_var {
	public:
	//domain is assumed to be {Domain[0],Domain[1],...,Domain[size]}
		~Categoric_domain() { free(Domain); };

		const float& operator[](const size_t& pos);
	private:
		float*      Domain;
	};




	class I_Potential {
		friend class I_Potential_Decorator;
	public:
		I_Potential(const I_Potential& to_copy) { abort(); };
		virtual ~I_Potential() {};

		struct I_Distribution_value {
			virtual ~I_Distribution_value() {};

			virtual void	Set_val(const float& v) = 0;
			virtual void	Get_val(float* result) = 0;
			virtual size_t* Get_indeces() = 0;
		};

		void Print_distribution(std::ostream& f, const bool& print_entire_domain = false); //when print_entire_domain is true, the entire domain is printed, even though the potential has a sparse distribution
		const std::list<Categoric_var*>*			Get_involved_var_safe() { return this->Get_involved_var(); };

		void			Find_Comb_in_distribution(std::list<float>* result,
			const std::list<size_t*>& comb_to_search, const std::list<Categoric_var*>& comb_to_search_var_order);

		static void Get_entire_domain(std::list<std::list<size_t>>* domain, const std::list<Categoric_var*>& Vars_in_domain);
	protected:
		I_Potential() {};

		//This version returns the value in the distribution of this pot. matching with the comb_to_search. When there is no match for 
		//a certain comb, a NULL is put into result. 
		static void			Find_Comb_in_distribution(std::list<I_Distribution_value*>* result,
			const std::list<size_t*>& comb_to_search, const std::list<Categoric_var*>& comb_to_search_var_order, I_Potential* pot);

		//This version returns for a partial combination (a combination involving a portion of the variables involved in this distribution), all the
		//values partially matching in the distribution of this pot. In case none of the value in the distribution of this pot matches, at least a NULL id put
		//into result
		static void			Find_Comb_in_distribution(std::list<I_Distribution_value*>* result,
			                size_t* partial_comb_to_search, const std::list<Categoric_var*>& partial_comb_to_search_var_order, I_Potential* pot);

		virtual std::list<Categoric_var*>*		    Get_involved_var() = 0;
		static std::list<Categoric_var*>*       Get_involved_var(I_Potential* pot) { return pot->Get_involved_var(); };

		virtual std::list<I_Distribution_value*>*	Get_distr() = 0;
		static std::list<I_Distribution_value*>*	Get_distr(I_Potential* pot) { return pot->Get_distr(); };

		virtual I_Potential*						Get_shape() = 0;
		static  I_Potential*						Get_shape(I_Potential* pot) { return pot->Get_shape(); };

		//computes all the indices constituting the domain of a group of categorical variables
		static void Get_entire_domain(std::list<size_t*>* domain, const std::list<Categoric_var*>& Vars_in_domain);
	};






	class I_Potential_Decorator : public I_Potential {
	public:
		~I_Potential_Decorator() { if (Destroy_wrapped) delete this->pwrapped; };
	protected:
		I_Potential_Decorator(I_Potential* to_wrap) :pwrapped(to_wrap), Destroy_wrapped(true) {};

		virtual std::list<Categoric_var*>*		Get_involved_var() { return pwrapped->Get_involved_var(); };
		virtual std::list<I_Distribution_value*>*	Get_distr() { return pwrapped->Get_distr(); };
		virtual I_Potential*						Get_shape() { return this->pwrapped->Get_shape(); };
	protected:
	// data
		bool			Destroy_wrapped; //when false, the wrapped potential is wrapped also in another decorator, and the destruction is done there
		I_Potential*	pwrapped;
	};


	class Potential_Shape : public I_Potential { //it's a concrete I_Potential
	public:
		Potential_Shape(const std::list<Categoric_var*>& var_involved); //A new shape potential is built, with all zeros as values for the domain
		Potential_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read);
		~Potential_Shape();

		void Import(const std::string& file_to_read);
		void Add_value(const std::list<size_t>& new_indeces, const float& new_val);
		void Set_ones();
		void Set_random(const float zeroing_threashold = 1.f);
	// reshape in order to have maximal value of the distribution equal to 1
		void Normalize_distribution();
	protected:
		void Check_add_value(const std::list<size_t>& indices); //check the specified combination is not already present in the distribution

		virtual std::list<Categoric_var*>*		    Get_involved_var() { return &this->Involved_var; };
		virtual std::list<I_Distribution_value*>*	Get_distr() { return &this->Distribution; };
		virtual I_Potential*						Get_shape() { return this; };
	private:
	// data
		std::list<Categoric_var*>			Involved_var;
		std::list<I_Distribution_value*>	Distribution;
	};


	class Potential_Exp_Shape : public I_Potential_Decorator {
	public:
		Potential_Exp_Shape(Potential_Shape* shape, const float& w = 1.f);
		Potential_Exp_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read, const float& w = 1.f);
		~Potential_Exp_Shape();

		struct Handler_weight {
		protected:
			static float* Get_weight(Potential_Exp_Shape* pot) { return &pot->mWeight; };
		};
	protected:
		virtual std::list<I_Distribution_value*>*	Get_distr() { return &this->Distribution; };
		void										Wrap(Potential_Shape* shape);
	// data
		float								    mWeight;
		std::list<I_Distribution_value*>		Distribution; //in this case are exponential value
	};


	class Potential : public I_Potential_Decorator {
	public:
		Potential(Potential_Shape* pot) : I_Potential_Decorator(pot) { };
		Potential(Potential_Exp_Shape* pot) : I_Potential_Decorator(pot) { };

		//merge a set of potentials referring to the same variabless. The density is created by computing the products of the factors in the merged set
		Potential(const std::list<Potential*>& potential_to_merge, const bool& use_sparse_format = true);

		//the potential is obtained by marginalizing the observed variable in potential passed as input
		Potential(const std::list<size_t>& val_observed, const std::list<Categoric_var*>& var_observed, Potential* pot_to_reduce);

		//obtain the marginal probabilities of the cluster of variables involved in this distribution, considering only this potential
		void Get_marginals(std::list<float>* prob_distr);
	protected:
		Potential() : I_Potential_Decorator(NULL) {};
	};



	class Message_Unary : public Potential { //adopted by belief propagation algorithms
	public:
		//for creating a message with all ones as values
		Message_Unary(Categoric_var* var_involved);

		//at first the unary potential are merged together, then the product of the binary and the merged potential is
		//obtained. Finally the variable present in the merge set is marginalized, adopting a sum or a MAP
		//exploited by message passing
		Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true);
		Message_Unary(Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP = true); //use this when the unary set is empty

		void Update(float* diff_to_previous , Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true);
		void Update(float* diff_to_previous, Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP = true);
	private:
		Potential_Shape* merge_binary_and_unary(Potential* binary_to_merge, Potential* unary, const bool& Sum_or_MAP);
	};

}

#endif