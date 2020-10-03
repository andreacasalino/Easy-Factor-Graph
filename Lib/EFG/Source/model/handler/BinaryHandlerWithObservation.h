#ifndef EFG_BINARY_HANDLER_OBS
#define EFG_BINARY_HANDLER_OBS

#include <model/ConditionalRandomField.h>

namespace EFG::model {

	class ConditionalRandomField::BinaryHandlerWithObservation : public GraphLearnable::AtomicHandler {
	public:
		BinaryHandlerWithObservation(pot::ExpFactor* pot, node::Node* Hidden_var, std::size_t* observed_val);
	private:
		float			GetBetaPart() override;
	// data
		node::Node*		Hidden;
		size_t*			Observation;
	// cache
		size_t			posObserved;
		size_t			posHidden;
	};

}

#endif