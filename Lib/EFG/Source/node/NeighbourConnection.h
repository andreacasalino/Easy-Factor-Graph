/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_NODE_CONNECTION_H__
#define __EFG_NODE_CONNECTION_H__

#include <node/Node.h>

namespace EFG::node {

	class Node::NeighbourConnection {
	public:
		static void initConnection(Node* peer_A, Node* peer_B, const pot::IPotential& pot_shared);

		inline const pot::IPotential* GetPot() const { return this->SharedPotential; };
		inline Node* GetNeighbour() const { return this->Neighbour; };
		inline const pot::IPotential* GetIncomingMessage() const { return this->Message2This.get(); };
		inline NeighbourConnection* GetLinked() const { return this->Linked; };
		inline const std::list<NeighbourConnection*>* GetNeighbourhood() { return &this->Neighbourhood; };

		void 							Disable();

		inline void						ResetOutgoingMessage() { this->Linked->Message2This.reset(); this->Neighbourhood.clear(); this->wasNeighbourhoodUpdated = false; };

		bool							isOutgoingRecomputationPossible();
		float							RecomputeOutgoing(const bool& Sum_or_MAP); //returns the difference 

		void							SetIncoming2Ones();
	private:
		NeighbourConnection() = default;
		NeighbourConnection(const NeighbourConnection&) = delete;
		void operator()(const NeighbourConnection&) = delete;

		inline void						UpdateNeighbourhood();

		class Message : public pot::IPotentialDecorator {
		public:
			/* Creates a Message with all 1 as values for the image
			*/
			Message(CategoricVariable* var_involved);
			/* Firstly, all potential_to_merge are merged together using Potential::Potential(potential_to_merge, false) obtaining a merged potential.
			* Secondly, the product of binary_to_merge and the merged potential is obtained.
			* Finally the message is obtained by marginalizing from the second product, the variable of potential_to_merge, adopting a sum or a MAP.
			* Exploited by message passing algorithms
			*/
			Message(const pot::IPotential& binary_pot, const std::vector<const pot::IPotential*>& unary_to_merge, const bool& Sum_or_MAP) { this->Update(binary_pot, unary_to_merge, Sum_or_MAP); };
			/* Same as Message_Unary::Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true),
			* but in the case potential_to_merge is empty
			*/
			Message(const pot::IPotential& binary_pot, CategoricVariable* var_to_marginalize, const bool& Sum_or_MAP) { this->Update(binary_pot, var_to_marginalize, Sum_or_MAP); };

			float Update(const pot::IPotential& binary_pot, const std::vector<const pot::IPotential*>& unary_to_merge, const bool& Sum_or_MAP); //returns the difference w.r.t. the previous message
			float Update(const pot::IPotential& binary_pot, CategoricVariable* var_to_marginalize, const bool& Sum_or_MAP); //returns the difference w.r.t. the previous message
		};

	// data
		Node*											Neighbour;
		std::unique_ptr<pot::IPotential> 				Message2This;       //nullptr when the message is not already available
		NeighbourConnection*							Linked;
		const pot::IPotential*							SharedPotential;
	// cache
		bool											wasNeighbourhoodUpdated;
		std::list<NeighbourConnection*>					Neighbourhood;
	};

}

#endif