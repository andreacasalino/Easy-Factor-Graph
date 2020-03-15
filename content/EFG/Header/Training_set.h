/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __CRF__TRAINING_SET_H__
#define __CRF__TRAINING_SET_H__

#include "Potential.h"

namespace EFG {

	/* \brief This class is used for describing a training set, i.e. a series
	 of combinations, not associated to a particular variable group objects,
	 but only to a set of variable names.
	 This object can be cast to a combinations object, using Get_as_combinations_list,
	 by passing the varible objects to consider as group.
	 */
	class I_Potential::combinations::_Training_set_t {
	public:
		/** \brief import the values of the set from a textual file.
		* \details The first row of such file must contain
		* the list of names of the variables involved in this set.
		* All other rows are a single sample of the set, reporting the values assumed by the variables,
		* with the order described by the first row
		*
		* @param[in] file_to_import file containing the set to import
		*/
		_Training_set_t(const std::string& file_to_import);

		/** \brief import the samples and the names directly.
		*
		* @param[in] samples the samples to consider for the training set
		* @param[in] var_names the names of the variables in the set
		*/
		_Training_set_t(const std::list<std::list<size_t>>& samples, const std::list<std::string>& var_names);

		/** \brief merge all the samples in the passed training sets.
		\details A consistency check is performed about all the variable names sets.
		*
		* @param[in] to_merge the training sets to merge
		*/
		_Training_set_t(const std::list<_Training_set_t*>& to_merge);

		~_Training_set_t() { free(this->Comb_allocated); };

		/** \brief get the training set as a combinations object.
		*
		* @param[in] variables_to_assume the varaibles to consider for creating the combinations object.
		*/
		I_Potential::combinations Get_as_combinations_list(const std::list<Categoric_var*>& variables_to_assume) const;
		
		/*!
		 * \brief This class describes a portion of a training set, obtained by sampling values in the original set. 
		 * Mainly used by stochastic gradient computation strategies.
		 */
		class subset : public Subject::Observer {
		public:
			/**
			* @param[in] set the training set from which this subset must be extracted
			* @param[in] size_percentage percentage to use for the extraction
			*/
			subset(_Training_set_t& set, const float& Sample_percentage = 1.f);

			~subset() { free(this->sampled_Comb_allocated); };

			/* \brief Resample the values to considr in the subset.
			*/
			void Resample();

			/* \brief The subset is cast as a combinations object, as similarly done by _Training_set_t::Get_as_combinations_list
			*/
			I_Potential::combinations Get_as_combinations_list(const std::list<Categoric_var*>& variables_to_assume) const;
		private:
		// data
			_Training_set_t*		pSource;
			size_t				sampled_Comb_allocated_size;
			size_t*				sampled_Comb_allocated;
		};

		/*!
		 * \brief This training set is reprinted in the location specified
		 * @param[in] file_name is the path of the file where the set must be printed
		 */
		void Print(const std::string& file_name) const;

	private:
		void			__Import(const std::list<std::list<size_t>>& samples);
		void			__Init_comb(combinations& combs, const std::list<Categoric_var*>& vars, const size_t& size, size_t* pcomb) const;
		void			__get_order(std::list<Categoric_var*>* result, const std::list<Categoric_var*>& vars) const;

	// data
		Subject						subset_subject;
		std::list<std::string>      Variable_names;
		size_t						Comb_allocated_size;
		size_t*						Comb_allocated;
	};
	typedef I_Potential::combinations::_Training_set_t Training_set;

}

#endif