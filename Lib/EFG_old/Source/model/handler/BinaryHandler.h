#ifndef EFG_BINARY_HANDLER
#define EFG_BINARY_HANDLER

#include <model/Learnable.h>

namespace EFG::model {

	class GraphLearnable::BinaryHandler : public GraphLearnable::AtomicHandler {
	public:
		BinaryHandler(GraphLearnable* model, pot::ExpFactor* pot_to_handle);
	private:
		float										GetBetaPart() override;
	// data
		node::Node* pNode1;
		node::Node* pNode2;
	};

}

#endif