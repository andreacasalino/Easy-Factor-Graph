//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_TRAINING_SET_H__
#define __CRF_TRAINING_SET_H__

#include "Potential.h"

namespace Segugio {

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
		* \brief Same as Training_set(const std::string& file_to_import), with the difference that after 
		* importing all the set, this latter is showed. Used mainly for debug
		* @param[in] file_to_import file containing the set to import
		* @param[in] display_after_parsing flag for specifying the import debuggin mode
		*/
		Training_set(const std::string& file_to_import, const bool& display_after_parsing); 
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
				static std::list<size_t*>*		Get_list(subset* sub_set) { return &sub_set->Sub_Set; };
				static std::list<std::string>*	Get_names(subset* sub_set) { return sub_set->pVariable_names; };
				static std::list<std::string>*	Get_names(Training_set* set) { return &set->Variable_names; };
			};

			//void Get_pos_of_var_in_set(std::list<size_t>* result, const std::list<Categoric_var*>& variables);
		private:
			std::list<std::string>*		pVariable_names;
			std::list<size_t*>			Sub_Set;
		};
	private:
	// data
		std::list<std::string>	Variable_names;
		std::list<size_t*>		Set;
	};

}

#endif