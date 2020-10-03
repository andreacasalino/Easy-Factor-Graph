/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_MODEL_CONDITIONAL_RANDOM_FIELD_H__
#define __EFG_MODEL_CONDITIONAL_RANDOM_FIELD_H__

#include <model/Learnable.h>
#include <node/belprop/BasicPropagator.h>

namespace EFG::model {

	/*!
	 * \brief This class describes Conditional Random fields
	 * \details Set_Observation_Set_var is depracated: the observed set of variables cannot be changed after construction.
	 */
	class ConditionalRandomField : public GraphLearnable {
	public:
		/** \brief The model is built considering the information contained in an xml configuration file.  @latexonly\label{CRF_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration file
		* @param[in] prefix to use. The file prefix_config_xml_file/config_xml_file is searched.
		*/
		ConditionalRandomField(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "", const node::bp::BeliefPropagator& propagator = node::bp::BasicStrategy());

		/** \brief Copy constructor.
		\details The same evidence set is assumed, considering as initial observations a set of null values.
		* @param[in] o the Conditional_Random_Field to copy
		*/
		ConditionalRandomField(const NodeFactory& o);

		/** \brief This constructor initializes the graph with the specified potentials passed as input
		, setting the variables passed as the one observed
		*
		* @param[in] shapes the initial set of potentials to insert (can be empty)
		* @param[in] constant_exp the initial set of constant (non tunable weights) exponential potentials to insert (can be empty)
		* @param[in] learnable_exp the clusters of initial tunable potentials to insert (can be empty). Each cluster is a collection
		of tunable exponential potentials sharing the same weight. A single cluster can have a single element, representing a normal tunable
		exponential potential to insert in the graph, which does not share the weight with anyone.
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called (this includes the passed potentials),
		* a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		* @param[in] observed_var the name of the variables to assume as evidences. It cannot be empty
		*/
		ConditionalRandomField(const Structure& strct, const std::vector<std::string>& observed_var, const bool& use_cloning_Insert = true, const node::bp::BeliefPropagator& propagator = node::bp::BasicStrategy());

		~ConditionalRandomField() { delete[] this->posObservationsTrainingSet; };

		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		inline void					  SetEvidences(const std::vector<size_t>& new_observed_vals) { this->NodeFactory::_SetEvidences(new_observed_vals); };
	private:
		class BinaryHandlerWithObservation;

		void 				 				 _Import(const Structure& strct, const std::vector<std::string>& evidences);

		std::vector<float>   				_GetBetaPart(const distr::Combinations& training_set) override;

		// cache for setting observations from training set
		const std::vector<CategoricVariable*>*			OrderTrainingSet;
		size_t*											posObservationsTrainingSet;
		size_t											posObservationsSize;
	};

}

#endif