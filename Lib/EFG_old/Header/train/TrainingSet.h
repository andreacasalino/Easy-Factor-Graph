/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG__TRAINING_SET_H__
#define __EFG__TRAINING_SET_H__

#include <distribution/Combinations.h>
#include <model/Learnable.h>

namespace EFG::train {

	/* \brief This class is used for describing a training set, i.e. a series
	 of combinations, not associated to a particular variable group objects,
	 but only to a set of variable names.
	 This object can be cast to a combinations object, using Get_as_combinations_list,
	 by passing the varible objects to consider as group.
	 */
	class TrainingSet : public sbj::MultiObservable {
	public:
		/** \brief import the values of the set from a textual file.
		* \details The first row of such file must contain
		* the list of names of the variables involved in this set.
		* All other rows are a single sample of the set, reporting the values assumed by the variables,
		* with the order described by the first row
		*
		* @param[in] file_to_import file containing the set to import
		*/
		TrainingSet(const std::string& file_to_import);

		/** \brief import the samples and the names directly.
		*
		* @param[in] samples the samples to consider for the training set
		* @param[in] var_names the names of the variables in the set
		*/
		template<typename Array>
		TrainingSet(const std::vector<std::string>& var_names, const std::list<Array>& combs);

		/** \brief add to this set all the combinations of the passed training set
		*/
		void operator+=(const TrainingSet& tr);

		~TrainingSet() { for(auto it= this->combs.begin(); it!=this->combs.end(); ++it) free(*it); };
		
		/*!
		 * \brief This class describes a portion of a training set, obtained by sampling values in the original set. 
		 * Mainly used by stochastic gradient computation strategies.
		 */
		class UsableTrainingSet : public Subject::Observer, public distr::Combinations {
		public:
			/**
			* @param[in] set the training set from which this subset must be extracted
			* @param[in] size_percentage percentage to use for the extraction
			*/
			UsableTrainingSet(TrainingSet& set, const model::GraphLearnable& model_to_train, const float& Sample_percentage = 1.f);

			~UsableTrainingSet();

			void Resample();
		private:
		// data
			TrainingSet*			source;
			size_t					originalSize;
		};

		/*!
		 * \brief This training set is reprinted in the location specified
		 * @param[in] file_name is the path of the file where the set must be printed
		 */
		void Print(const std::string& file_name) const;

	private:
		void 			__Import(const std::vector<std::string>& var_names, const std::list<std::pair<size_t* , size_t>>& combs_raw);
	// data
		std::list<size_t*>			combs;
		std::vector<std::string>	varNames;
		std::vector<size_t>			maxCombVals;
	};

	template<typename Array>
	TrainingSet::TrainingSet(const std::vector<std::string>& var_names, const std::list<Array>& combs){

		size_t comb_size = var_names.size();
		std::list<std::pair<size_t* , size_t>> combs_raw;
		size_t k;
		for(auto it = combs.begin(); it!=combs.end(); ++it){
			size_t* c = (size_t*)malloc(comb_size * sizeof(size_t));
			for(k=0; k<comb_size; ++k) c[k] = (*it)[k];
			std::pair<size_t* , size_t> temp;
			temp.first = c;
			temp.second = comb_size;
			combs_raw.emplace_front(temp);
		}
		this->__Import(var_names, combs_raw);

	}

}

#endif