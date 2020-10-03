#ifndef EFG_UNARY_HANDLER
#define EFG_UNARY_HANDLER

#include <model/Learnable.h>

namespace EFG::model {

	class GraphLearnable::UnaryHandler : public GraphLearnable::AtomicHandler {
	public:
		UnaryHandler(GraphLearnable* model, pot::ExpFactor* pot_to_handle) : AtomicHandler(pot_to_handle), pNode(model->_FindNode(pot_to_handle->GetDistribution().GetVariables().front()->GetName())) {};
	private:
		float										GetBetaPart() override;
	// data
		node::Node* pNode;
	};

}

#endif