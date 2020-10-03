/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_MODEL_GRAPH_H__
#define __EFG_MODEL_GRAPH_H__

#include <node/NodeFactory.h>
#include <node/belprop/BasicPropagator.h>

namespace EFG::model {

	/*!
	 * \brief Interface for managing generic graphs.
	 * \details Both Exponential and normal shapes can be included
	 * into the model. Learning is not possible: all belief propagation
	 * operations are performed assuming the mdoel as is.
	 * Every Potential_Shape or Potential_Exp_Shape is copied and that copy is
	 * inserted into the model.
	 */
	class Graph : public node::Node::NodeFactory {
	public:
		/** \brief empty constructor
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called, a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		*/
		Graph(const bool& use_cloning_Insert = true, const node::bp::BeliefPropagator& propagator = node::bp::BasicStrategy()) : node::Node::NodeFactory(use_cloning_Insert, propagator) {};
		/** \brief The model is built considering the information contained in an xml configuration file. @latexonly\label{Graph_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration file
		* @param[in] prefix to use. The file prefix_config_xml_file/config_xml_file is searched.
		*/
		Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "", const node::bp::BeliefPropagator& propagator = node::bp::BasicStrategy());

		/** \brief Copy constructor.
		\details All the potentials of the graph to copy are absorbed, by copying it.
		Also all the additional future potentials inserted will be copied.
		* @param[in] o the Graph to copy
		*/
		Graph(const node::Node::NodeFactory& o) : node::Node::NodeFactory(true, *_GetPropagator(o)) { this->_Insert(o.GetStructure()); };

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		*/
		inline void Insert(pot::Factor& pot) { this->_Insert(&pot); };

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		*/
		inline void Insert(pot::ExpFactor& pot) { this->_Insert(&pot); };
		/** \brief Absorbs all the variables and the potentials contained in the model passed as input.
		* \details The model receiving the potentials must be defined to do copying insertion.
		Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed.
		*/
		inline void	Insert(const Structure& strct) { this->_Insert(strct); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		inline void	SetEvidences(const std::vector<std::pair<std::string, size_t>>& new_observations) { this->NodeFactory::_SetEvidences(new_observations); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		inline void	SetEvidences(const std::vector<size_t>& new_observations) { this->NodeFactory::_SetEvidences(new_observations); };
	};

}

#endif