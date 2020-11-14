/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <train/Trainer.h>
#include <cmath>
using namespace std;

//#define COMPILE_BFGS

#ifdef COMPILE_BFGS
#include <Eigen/Dense>
using namespace Eigen;
#endif

namespace EFG::train {

	class Trainer::FixedStep : public Trainer::AdvancerStrategy {
	public:
		FixedStep(const float& step) : Step_to_advance(step) {};
	private:
		void Reset() override {};
		float advance(model::GraphLearnable* model_to_advance, const train::TrainingSet::UsableTrainingSet& train_set, const bool& force_alfa_rec) override;
	// data
		float Step_to_advance;
	};

#ifdef COMPILE_BFGS
	class Trainer::BFGS : public Trainer::AdvancerStrategy {
	public:
		BFGS() : pData(NULL) {};
		~BFGS() { this->Reset(); };

		void Reset();
	private:
		float advance(Graph_Learnable* model_to_advance,const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) override;
		void __line_search(Graph_Learnable* model_to_advance, VectorXf& S, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);

		struct Cache {
			MatrixXf invB;
			VectorXf Grad_old;
		};
	// data
		Cache*		pData;
	};
#endif



	float Trainer::LikelihhodEvaluator::operator()(const train::TrainingSet::UsableTrainingSet& set){

		vector<const std::size_t*> temp;
		temp.reserve(set.size());
		auto it = set.getIter();
		itr::forEach<distr::Combinations::constIterator>(it, [&temp](distr::Combinations::constIterator& itt) { temp.push_back(*itt); });
		return this->Evaluator.GetTrainingSetLikelihood(temp);

	}



	class Trainer::EntireSet : public Trainer {
	public:
		EntireSet(std::unique_ptr<AdvancerStrategy> to_wrap) : Trainer(std::move(to_wrap)) {};
		void operator()(model::GraphLearnable& model_to_train, train::TrainingSet& training_set, const unsigned int& Max_Iterations, std::list<float>* descend_story) override;
	};

	class Trainer::StochSet : public Trainer {
	public:
		StochSet(std::unique_ptr<AdvancerStrategy> to_wrap, const float& percentage_to_use) : Trainer(std::move(to_wrap)), Percentage(percentage_to_use) {};
		void operator()(model::GraphLearnable& model_to_train, train::TrainingSet& training_set, const unsigned int& Max_Iterations , std::list<float>* descend_story) override;
	private:
		float Percentage;
	};




	void Trainer::EntireSet::operator()(model::GraphLearnable& model_to_train, train::TrainingSet& training_set, const unsigned int& Max_Iterations , std::list<float>* descend_story) {

		if (model_to_train.GetModelSize() == 0) throw 0;
		if (descend_story != nullptr) descend_story->clear();

		this->advStrtgy->Reset();
		float variation;
		train::TrainingSet::UsableTrainingSet Set(training_set, model_to_train);
		unique_ptr<Trainer::LikelihhodEvaluator> lklhd_eval;
		if(descend_story != nullptr) lklhd_eval = std::make_unique<LikelihhodEvaluator>(model_to_train);
		for (unsigned int k = 0; k < Max_Iterations; ++k) {
			variation = this->advStrtgy->advance(&model_to_train, Set, false);
			if (descend_story != nullptr)  descend_story->push_back((*lklhd_eval)(Set));
			if (variation <= 1e-3) break;
		}

	}

	void Trainer::StochSet::operator()(model::GraphLearnable& model_to_train, train::TrainingSet& training_set, const unsigned int& Max_Iterations , std::list<float>* descend_story) {

		if (model_to_train.GetModelSize() == 0) throw 0;
		if (descend_story != nullptr) descend_story->clear();

		this->advStrtgy->Reset();
		float variation;
		train::TrainingSet::UsableTrainingSet Set(training_set, model_to_train, this->Percentage);
		unique_ptr<Trainer::LikelihhodEvaluator> lklhd_eval;
		if(descend_story != nullptr) lklhd_eval = std::make_unique<LikelihhodEvaluator>(model_to_train);
		for (unsigned int k = 0; k < Max_Iterations; ++k) {
			if(k > 0) Set.Resample();
			variation = this->advStrtgy->advance(&model_to_train, Set, true);
			if (descend_story != nullptr)  descend_story->push_back((*lklhd_eval)(Set));
			if (variation <= 1e-3) break;
		}

	}

	float Trainer::FixedStep::advance(model::GraphLearnable* model_to_advance, const train::TrainingSet::UsableTrainingSet& train_set, const bool& force_alfa_rec) {

		vector<float> grad_att = model_to_advance->GetTunableGrad(train_set, force_alfa_rec);

		vector<float> w_att = model_to_advance->GetTunable();

		float delta_w = 0.f, temp;
		size_t k, K = grad_att.size();
		for (k = 0; k < K; ++k) {
			temp = this->Step_to_advance * grad_att[k];
			w_att[k] += temp;
			temp = abs(temp);
			if (temp > delta_w) delta_w = temp;
		}
		model_to_advance->SetTunable(w_att);

		return delta_w;

	}

#ifdef COMPILE_BFGS
	void list_2_VectorXf(VectorXf* result, const list<float>& L) {

		*result = VectorXf(L.size());
		size_t k = 0;
		for (auto it = L.begin(); it != L.end(); ++it) {
			(*result)(k) = *it;
			++k;
		}

	}

	void BFGS::__line_search(Graph_Learnable* model_to_advance, VectorXf& S, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {
 
		list<float> w_initial, w_new;
		Graph_Learnable::Weights_Manager::Get_tunable_w(&w_initial, model_to_advance);
		list<float>::iterator it_w, it_w2;
		for (it_w = w_initial.begin(); it_w != w_initial.end(); ++it_w)
			w_new.push_back(1.f);
		list<size_t*>::const_iterator it_comb;

		float E_old = 0.f, E_new;
		float temp;
		for (it_comb = comb_in_train_set.begin(); it_comb != comb_in_train_set.end(); ++it_comb) {
			model_to_advance->Eval_Log_Energy_function_normalized(&temp, *it_comb, comb_var);
			E_old += temp;
		}

		size_t k2;
		for (size_t k = 0; k < 5; ++k) {
			S *= 0.5f;
			it_w2 = w_new.begin();
			k2 = 0;
			for (it_w = w_initial.begin(); it_w != w_initial.end(); ++it_w) {
				*it_w2 = *it_w + S(k2);
				++k2;
				++it_w2;
			}
			this->Set_w(w_new, model_to_advance);

			E_new = 0.f;
			for (it_comb = comb_in_train_set.begin(); it_comb != comb_in_train_set.end(); ++it_comb) {
				model_to_advance->Eval_Log_Energy_function_normalized(&temp, *it_comb, comb_var);
				E_new += temp;
			}

			if (E_new < E_old) {
				S *= 2.f;
				break;
			}
			E_old = E_new;
		}

		it_w2 = w_new.begin();
		k2 = 0;
		for (it_w = w_initial.begin(); it_w != w_initial.end(); ++it_w) {
			*it_w2 = *it_w + S(k2);
			++k2;
			++it_w2;
		}
		this->Set_w(w_new, model_to_advance);

	}

	float BFGS::_advance(Graph_Learnable* model_to_advance, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {

		if (this->pData == NULL) {
			size_t model_size = model_to_advance->Get_model_size();
			this->pData = new Cache();

			this->pData->invB = MatrixXf::Identity(model_size, model_size);
			
			list<float> temp;
			this->Get_w_grad(model_to_advance, &temp, comb_in_train_set, comb_var);
			list_2_VectorXf(&this->pData->Grad_old, temp);
		}

		VectorXf S = this->pData->invB * this->pData->Grad_old; 
		this->__line_search(model_to_advance, S, comb_in_train_set, comb_var);

		list<float> grad_att;
		this->Get_w_grad(model_to_advance, &grad_att, comb_in_train_set, comb_var);
		VectorXf Grad_att;
		list_2_VectorXf(&Grad_att, grad_att);

		VectorXf Y = Grad_att - this->pData->Grad_old;
		this->pData->Grad_old = Grad_att;

		float dot_temp = Y.dot(S);
		MatrixXf temp = (1.f / dot_temp) * S * Y.transpose();
		temp *= -1.f;
		size_t k;
		for (k = 0; k < (int)temp.cols(); ++k)
			temp(k, k) += 1.f;

		this->pData->invB = temp * this->pData->invB * temp.transpose();
		this->pData->invB += (1.f / dot_temp) * S * S.transpose();


		//EigenSolver<MatrixXf> solver(this->pData->invB);
		//VectorXcf eig_vals = solver.eigenvalues();
		//float max_eig = abs(eig_vals(0).real()), eig_temp;
		//for (k = 1; k < (size_t)eig_vals.size(); ++k) {
		//	eig_temp = abs(eig_vals(k).real());
		//	if (eig_temp > max_eig)
		//		max_eig = eig_temp;
		//}
		//return max_eig;

		float min_S = abs(S(0)), temp_S;
		for (k = 1; k < (size_t)S.size(); ++k) {
			temp_S = abs(S(k));
			if (S(k) < temp_S)
				min_S = S(k);
		}
		return 1000.f;

	}

	void BFGS::Reset() {

		if (this->pData != NULL)
			delete this->pData;
		this->pData = NULL;

	}

#endif



	std::unique_ptr<Trainer> Trainer::GetFixedStep(const float& step_size, const float& stoch_grad_percentage) {

		std::unique_ptr<AdvancerStrategy> created = make_unique<FixedStep>(step_size);
		if (stoch_grad_percentage == 1.f) return make_unique<EntireSet>(std::move(created));
		else							  return make_unique<StochSet>(std::move(created), stoch_grad_percentage);

	}

#ifdef COMPILE_BFGS
	I_Trainer* I_Trainer::Get_BFGS(const float& stoch_grad_percentage) {

		std::unique_ptr<AdvancerStrategy> created = make_unique<BFGS>();
		if (stoch_grad_percentage == 1.f) return make_unique<EntireSet>(std::move(created));
		else							  return make_unique<StochSet>(std::move(created), stoch_grad_percentage);

	}
#endif

}

