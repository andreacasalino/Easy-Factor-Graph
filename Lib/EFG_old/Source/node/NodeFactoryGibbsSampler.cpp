#include <node/NodeFactory.h>
#include <ctime>
#include "NeighbourConnection.h"
using namespace std;

namespace EFG::node {

	std::list<std::vector<size_t>> Node::NodeFactory::GibbsSamplingHiddenSet(const unsigned int& N_samples, const unsigned int& initial_sample_to_skip, const int& seed) {

		this->__RecomputeClusters();

		if (seed >= 0) srand((unsigned int)seed);
		else srand((unsigned int)time(NULL));

		struct DiscreteSampler {
			size_t  operator()(const vector<float>& distr) const {
				float r = (float)rand() / (float)RAND_MAX;
				size_t k = 0;
				if (r <= distr[k])  return k;

				float cum = distr[k];
				size_t K = distr.size();
				for (k = k + 1; k < K; ++k) {
					cum += distr[k];
					if (r <= cum) return k;
				}

				return (distr.size() - 1);
			};

			size_t  operator()(const std::size_t& size) const {
				return (*this)(vector<float>(size, 1.f / (float)size));
			};
		};

		class Sampler {
			struct NodeInfo {
				CategoricVariable*														  var;
				size_t*																	  sample;
				vector<const pot::IPotential*>											  unaries;
				vector<tuple<const pot::IPotential*, CategoricVariable*, const std::size_t*>>  neighbour;
			};

			// data
			size_t* Sampled;
			vector<NodeInfo>	Info;
			DiscreteSampler	    disc_sampler;
		public:
			Sampler(NodeFactory* model) {
				auto H_vars = model->GetHiddenSet();
				this->Sampled = new size_t[H_vars.size()];
				this->Info.reserve(H_vars.size());

				size_t H = H_vars.size(), p;
				for (p = 0; p < H; ++p) this->Sampled[p] = 0;
				for (size_t h = 0; h < H; ++h) {
					this->Info.emplace_back();
					this->Info.back().var = H_vars[h];
					this->Info.back().sample = &this->Sampled[h];

					Node* node = model->_FindNode(H_vars[h]->GetName());
					auto perm = node->GetPermanent();
					auto temp = node->GetTemporary();
					this->Info.back().unaries.reserve(perm->size() + temp->size());
					for (auto it = perm->begin(); it != perm->end(); ++it) this->Info.back().unaries.push_back(*it);
					for (auto it = temp->begin(); it != temp->end(); ++it) this->Info.back().unaries.push_back(&(*it));

					auto Neigh = node->GetActiveConnections();
					this->Info.back().neighbour.reserve(Neigh->size());
					for (auto itN = Neigh->begin(); itN != Neigh->end(); ++itN) {
						const std::size_t* ref = nullptr;
						for (p = 0; p < H; ++p) {
							if ((*itN)->GetNeighbour()->GetVar() == H_vars[p]) {
								ref = &this->Sampled[p];
								break;
							}
						}
						this->Info.back().neighbour.emplace_back(move(make_tuple((*itN)->GetPot(), (*itN)->GetNeighbour()->GetVar(), ref)));
					}
				}
			};
			
			~Sampler() { delete[] this->Sampled; };

			inline const std::size_t*	getSampled() const { return this->Sampled; };
			size_t					size()  const { return this->Info.size(); };
			void 					operator()(const unsigned int& iterations) {
				size_t i, I = this->Info.size();
				size_t n, N;
				for (unsigned int s = 0; s < iterations; ++s) {
					for (i = 0; i < I; ++i) {
						vector<const pot::IPotential*> to_merge;
						list<pot::Factor> temp;
						to_merge.reserve(this->Info[i].unaries.size() + this->Info[i].neighbour.size());
						to_merge = this->Info[i].unaries;
						N = this->Info[i].neighbour.size();
						for (n = 0; n < N; ++n) {
							temp.emplace_back(std::vector<size_t>{*get<2>(this->Info[i].neighbour[n])}, std::vector<CategoricVariable*>{get<1>(this->Info[i].neighbour[n])}, * get<0>(this->Info[i].neighbour[n]));
							to_merge.push_back(&temp.back());
						}

						if (to_merge.empty())    *this->Info[i].sample = disc_sampler(this->Info[i].var->size());
						else   					*this->Info[i].sample = disc_sampler(pot::Factor(to_merge, false).GetMarginals());
					}
				}
			};
		} sampler(this);

		unsigned int n_delta = (int)floorf(initial_sample_to_skip * 0.1f);
		if (n_delta == 0) n_delta = 1;

		//skip initial samples
		sampler(initial_sample_to_skip);

		//do sampling
		list<vector<size_t>> result;
		size_t v, V = sampler.size();
		auto Samples = sampler.getSampled();
		for (size_t n = 0; n < N_samples; ++n) {
			sampler(n_delta);
			result.emplace_back();
			result.back().reserve(V);
			for (v = 0; v < V; ++v) result.back().push_back(Samples[v]);
		}
		return result;

	}

}

