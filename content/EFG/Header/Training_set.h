/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __CRF_TRAINING_SET_H__
#define __CRF_TRAINING_SET_H__

#include "Potential.h"

namespace EFG {

std::list<std::string> extract_names(const std::list<Categoric_var*>& variable_in_the_net);


	/*!
	 * \brief This class is used for describing a training set for a graph.
	 * \details A set is described in a textual file, where the first row must contain
	 * the list of names of the variables (all the variables) constituting a graph.
	 * All other rows are a single sample of the set, reporting the values assumed by the variables,
	 * with the order described by the first row
	 */
	class Training_set {
	public:
		/**
		* @param[in] file_to_import file containing the set to import
		*/
		Training_set(const std::string& file_to_import);

		/**
		* \brief This class is adopted for parsing a set of samples to import as a novel training set.
		* You have to derive yout custom extractor, implementing the two vritual method
		*/
		template<typename Array>
		class I_Extractor {
		public:
			virtual const size_t& get_val_in_pos(const Array& container, const size_t& pos) = 0;
			virtual size_t				get_size(const Array& container) = 0;
		};
		/**
		* \brief Similar to Training_set(const std::string& file_to_import), 
		* \details with the difference that the training set is not red from a textual file
		* but it is imported from a list of container (generic can be list, vector or other) 
		* describing the samples of the set. You have to derived your own extractor for managing 
		* your particular container. Basic_Extractor is a baseline extractor that can be used for
		* all those type having the method size() and the operator[].
		* @param[in] variable_names the ordered list of variables to assume for the samples
		* @param[in] samples the list of generic Array representing the samples of the training set
		* @param[in] extractor the particular extractor to use, see I_Extractor
		*/
		template<typename Array>
		Training_set(const std::list<std::string>& variable_names, std::list<Array>& samples, I_Extractor<Array>* extractor) : 
			Variable_names(variable_names), Is_training_set_valid(true) {

			this->Is_training_set_valid = false;

			if(variable_names.empty() || samples.empty())  throw 0; //empty training set

			size_t vec_size = variable_names.size(), k;
			size_t* temp;
			for (auto it = samples.begin(); it != samples.end(); it++) {
				if (extractor->get_size(*it) != vec_size) throw 1; // inconsistent training set data

				temp = (size_t*)malloc(sizeof(size_t) * vec_size);
				for (k = 0; k < vec_size; k++)
					temp[k] = extractor->get_val_in_pos(*it, k);

				this->Set.push_back(temp);
			}

			this->Is_training_set_valid = true;

		};
		/**
		* \brief Same as Training_set(const std::list<std::string>& variable_names, std::list<Array> samples, I_Extractor<Array>* extractor)
		* passing the variables involved instead of the names
		*/
		template<typename Array>
		Training_set(const std::list<Categoric_var*>& variable_in_the_net, std::list<Array>& samples, I_Extractor<Array>* extractor) :
			Training_set(extract_names(variable_in_the_net), samples, extractor) {};

		/**
		* \brief Basic extractor, see Training_set(const std::list<std::string>& variable_names, std::list<Array> samples, I_Extractor<Array>* extractor)
		*/
		template<typename Array>
		class Basic_Extractor : public I_Extractor<Array> {
			const size_t& get_val_in_pos(const Array& container, const size_t& pos) { return container[pos]; };
			size_t				get_size(const Array& container) { return container.size(); };
		};

		~Training_set();
		
		/*!
		 * \brief This class is describes a portion of a training set, obtained by sampling values in the original set. 
		 * Mainly used by stochastic gradient computation strategies
		 */
		struct subset {
		public:
			/**
			* @param[in] set the training set from which this subset must be extracted
			* @param[in] size_percentage percentage to use for the extraction
			*/
			subset(Training_set* set, const float& size_percentage = 1.f);

			struct Handler {
			protected:
				static std::list<size_t*>*		Get_list(subset* sub_set) { if (!sub_set->Is_sub_set_valid) return NULL; else  return &sub_set->Sub_Set; };
				static std::list<std::string>*	Get_names(subset* sub_set) { if (!sub_set->Is_sub_set_valid) return NULL; else   return sub_set->pVariable_names; };
				static std::list<std::string>*	Get_names(Training_set* set) { if (!set->Is_training_set_valid) return NULL; else   return &set->Variable_names; };
			};

			//void Get_pos_of_var_in_set(std::list<size_t>* result, const std::list<Categoric_var*>& variables);
			const bool& Get_validity() { return this->Is_sub_set_valid; };
		private:
			bool									Is_sub_set_valid; //inherited form the originating training set
			std::list<std::string>*		pVariable_names;
			std::list<size_t*>				Sub_Set;
		};

		/*!
		 * \brief This training set is reprinted in the location specified
		 * @param[in] file_name is the path of the file where the set must be printed
		 */
		void Print(const std::string& file_name);

		/*!
		 * \brief Returns true in case this set can be used for performing the training of a model, otherwise is false
		 */
		const bool& Get_validity() { return this->Is_training_set_valid; };
	private:
	// data
		bool								Is_training_set_valid;
		std::list<std::string>	Variable_names;
		std::list<size_t*>			Set;
	};

}

#endif