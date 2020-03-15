/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __CRF_SUBGRAPH_H__
#define __CRF_SUBGRAPH_H__

#include "Node.h"
#include <memory>


namespace EFG {

	/** \brief This class describes sub-group of variables inserted in a bigger model.
	\details Every time a call to Get$\_$joint$\_$marginal$\_$distribution or Get$\_$message$\_$from$\_$outside is done, 
	the current evidences of the observed model, as well as the current contained potentials, are considered.
	Such operations are not thread safe.
	*/
	class Node::Node_factory::_SubGraph_t {
	public:
		/** \brief Builds a reduction of a net, considering the passed group of variables.
		* @param[in] variables the variables constituting the sub-group. Such variables must be contained 
		in the passed model, otherwise an exception is raised.
		 */
		_SubGraph_t(Node_factory& model, const std::list<Categoric_var*>& variables);

		/** \brief Similar to $\_$SubGraph$\_$t(Node$\_$factory& model, const std::list<Categoric$\_$var*>& variables), but passing a subgraph already built.
		* @param[in] variables the variables constituting the sub-group. Such variables must be contained 
		in the passed sub-graph.
		 */
		_SubGraph_t(_SubGraph_t& subgraph_to_reduce, const std::list<Categoric_var*>& variables);

		/** \brief Returns all the variables constituting a sub-graph.
		 */
		void							Get_All_variables_in_model(std::list<Categoric_var*>* result);

		/** \brief Returns the joint marginal distribution of the sub-group, conditioned to the evidences set in 
		the model from which this sub-group was extracted.
		* @param[in] return Is a Potential$\_$Shape whose images are the marginal joint probabilities
		 */
		std::unique_ptr<Potential_Shape> Get_joint_marginal_distribution();

		/** \brief Returns a unary factor reducing all the messages incoming from the rest of the model toward this node in the subgraph.
		 */
		void							Get_message_from_outside(std::vector<float>* message, Categoric_var* var);
	private:	
		void				__Get_outside_inside(Node* involved, std::list<Node::Neighbour_connection*>* outside, std::list<Node::Neighbour_connection*>* inside);
		observation_info*	__Check_is_observed(Node* involved);
	// data
		Subject::Observer	obs;
		Node_factory*		source;
		std::list<Node*>    Nodes;
	};
	typedef Node::Node_factory::_SubGraph_t  SubGraph;

}

#endif