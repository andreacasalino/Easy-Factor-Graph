/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_MODEL_RANDOM_FIELD_H__
#define __EFG_MODEL_RANDOM_FIELD_H__

#include <model/Learnable.h>

namespace EFG::model {

	/*!
	 * \brief This class describes a generic Random Field, not having a particular set of variables observed.
	 */
	class RandomField : public GraphLearnable {
	public:
		/** \brief empty constructor
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called, a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		*/
		RandomField(const bool& use_cloning_Insert = true) : GraphLearnable(use_cloning_Insert) {};

		/** \brief The model is built considering the information contained in an xml configuration file. @latexonly\label{RF_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration the file to import (can be simply a file name, a relative path or an absolute path)
		*/
		RandomField(const std::string& config_xml_file);

		/** \brief Copy constructor.
		\details All the potentials of the graph to copy are absorbed, by copying it.
		Also all the additional future potentials inserted will be copied.
		* @param[in] o the Random_Field to copy
		*/
		RandomField(const NodeFactory& o) : GraphLearnable(true) { this->_Copy(o); };

		/*!
		 * \brief Similar to Graph::Insert(Potential_Shape* pot)
		 */
		inline void Insert(pot::Factor& pot) { this->NodeFactory::_Insert(pot); };
		/*!
		 * \brief Similar to Graph::Insert(Potential_Exp_Shape* pot).
		* @param[in] is_weight_tunable When true, you are specifying that this potential has a weight learnable, otherwise the value
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		* of the weight is assumed constant.
		 */
		inline void Insert(pot::ExpFactor& pot, const bool& is_weight_tunable = true) { this->GraphLearnable::_Insert(pot, is_weight_tunable); };
		/*!
		 * \brief Insert a tunable exponential shape, whose weight is shared with another already inserted tunable shape.
		 * \details This allows having many exponential tunable potetials which share the value of the weight: this is automatically account for when
		 * performing learning.
		* @param[in] vars_of_pot_whose_weight_is_to_share the list of varaibles involved in a potential already inserted whose weight is to share with the
		* potential passed. They must be references to the variables actually wrapped into the model.
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		* of the weight is assumed constant.
		 */
		void Insert(pot::ExpFactor& pot, const std::vector<std::string>& vars_of_pot_whose_weight_is_to_share);

		inline void InsertMove(pot::Factor&& pot) { this->NodeFactory::_Insert(std::move(pot)); };

		inline void InsertMove(pot::ExpFactor&& pot, const bool& is_weight_tunable = true) { this->GraphLearnable::_Insert(std::move(pot), is_weight_tunable); };

		void InsertMove(pot::ExpFactor&& pot, const std::vector<std::string>& vars_of_pot_whose_weight_is_to_share);

		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		inline void					  SetEvidences(const std::vector<std::pair<std::string, size_t>>& new_observations) { this->NodeFactory::_SetEvidences(new_observations); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		inline void					  SetEvidences(const std::vector<size_t>& new_observations) { this->NodeFactory::_SetEvidences(new_observations); };

		/** \brief Absorbs all the variables and the potentials contained in the model passed as input.
		* \details The model receiving the potentials must be defined to do copying insertion.
		Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed.
		*/
		inline void					Insert(const Structure& strct, const bool& use_move) { this->GraphLearnable::_Insert(strct, use_move); };
	private:
		std::vector<float>   		 _GetBetaPart(const distr::Combinations& training_set) override;
	};

}

#endif