/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_NODE_H__
#define __EFG_NODE_H__

#include <CategoricVariable.h>
#include <potential/Factor.h>

namespace EFG::node {

	class Node {
	public:
		~Node();

		class NeighbourConnection;

		class NodeFactory;

		inline CategoricVariable*							GetVar() const { return this->Variable; };
		inline const std::list<const pot::IPotential*>*		GetPermanent() const { return &this->PermanentUnary; };
		inline const std::list<pot::Factor>*			    GetTemporary() const { return &this->TemporaryUnary; };

		/** \brief returns in a unique collection: the permanent potentials, the temporary potentials and all the messages incoming from the other nodess 
		*/
		std::vector<const pot::IPotential*>					GetAllUnaries() const;

		inline const std::list<NeighbourConnection*>* 		GetActiveConnections() const { return &this->ActiveConnections; };
	private:
		Node(CategoricVariable* var, const bool& dont_clone_var);

		Node(const Node&) = delete;
		void operator()(const Node& ) = delete;
	// data												
		CategoricVariable*								 Variable;
		std::list<const pot::IPotential*>				 PermanentUnary;
		std::list<pot::Factor>					 		 TemporaryUnary; //those coming from marginalizing some observed variables
		std::list<NeighbourConnection*>					 ActiveConnections;
		std::list<NeighbourConnection*>					 DisabledConnections;
	};

}

#endif
