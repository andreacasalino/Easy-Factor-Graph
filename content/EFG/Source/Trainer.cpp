/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Trainer.h"
#include <cmath>
using namespace std;

#ifdef COMPILE_BFGS
#include <Eigen/Dense>
using namespace Eigen;
#endif

namespace EFG {

	class I_Trainer::Fixed_step : public I_Trainer::I_Advancer_Strategy {
	public:
		Fixed_step(const float& step) : Step_to_advance(step) {};
	private:
		virtual void Reset() {};
		float advance(Graph_Learnable* model_to_advance, const I_Potential::combinations& train_set, const bool& force_alfa_rec);
	// data
		float Step_to_advance;
	};

#ifdef COMPILE_BFGS
	class I_Trainer::BFGS : public Advancer_Concrete {
	public:
		BFGS() : pData(NULL) {};
		~BFGS() { this->Reset(); };

		void Reset();
	private:
		float advance(Graph_Learnable* model_to_advance,const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);
		void __line_search(Graph_Learnable* model_to_advance, VectorXf& S, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);

		struct Cache {
			MatrixXf invB;
			VectorXf Grad_old;
		};
	// data
		Cache*		pData;
	};
#endif




	class I_Trainer::Entire_Set : public I_Trainer {
	public:
		Entire_Set(I_Trainer::I_Advancer_Strategy* to_wrap) : I_Trainer(to_wrap) {};
		void Train(Graph_Learnable& model_to_train, Training_set& training_set, const unsigned int& Max_Iterations, std::list<float>* descend_story);
	};

	class I_Trainer::Stoch_Set : public I_Trainer {
	public:
		Stoch_Set(I_Trainer::I_Advancer_Strategy* to_wrap, const float& percentage_to_use) : I_Trainer(to_wrap), Percentage(percentage_to_use) {};
		void Train(Graph_Learnable& model_to_train, Training_set& training_set, const unsigned int& Max_Iterations , std::list<float>* descend_story);
	private:
		float Percentage;
	};




	void I_Trainer::Entire_Set::Train(Graph_Learnable& model_to_train, Training_set& training_set, const unsigned int& Max_Iterations , std::list<float>* descend_story) {

		if (model_to_train.Get_model_size() == 0) throw 0;
		if (descend_story != NULL) descend_story->clear();

		this->adv_strtgy->Reset();
		float variation, temp;
		list<Categoric_var*> vars;
		model_to_train.Get_All_variables_in_model(&vars);
		auto Set = training_set.Get_as_combinations_list(vars);
		for (unsigned int k = 0; k < Max_Iterations; k++) {
			variation = this->adv_strtgy->advance(&model_to_train, Set, false);
			if (descend_story != NULL) {
				model_to_train.Get_TrainingSet_Likelihood(&temp , Set);
				descend_story->push_back(temp);
			}
			if (variation <= 1e-3) break;
		}

	}

	void I_Trainer::Stoch_Set::Train(Graph_Learnable& model_to_train, Training_set& training_set, const unsigned int& Max_Iterations , std::list<float>* descend_story) {

		if (model_to_train.Get_model_size() == 0) throw 0;
		if (descend_story != NULL) descend_story->clear();

		this->adv_strtgy->Reset();
		float variation;
		list<Categoric_var*> vars;
		model_to_train.Get_All_variables_in_model(&vars);
		Training_set::subset sub_set( training_set , this->Percentage );
		for (unsigned int k = 0; k < Max_Iterations; k++) {
			sub_set.Resample();
			I_Potential::combinations Set = sub_set.Get_as_combinations_list(vars);
			variation = this->adv_strtgy->advance(&model_to_train, Set, true);
			if (descend_story != NULL) {
				float temp;
				model_to_train.Get_TrainingSet_Likelihood(&temp, Set);
				descend_story->push_back(temp);
			}

			if (variation <= 1e-3) break;
		}

	}

	float I_Trainer::Fixed_step::advance(Graph_Learnable* model_to_advance, const I_Potential::combinations& train_set, const bool& force_alfa_rec) {

		vector<float> grad_att;
		model_to_advance->Get_tunable_grad(&grad_att, train_set, force_alfa_rec);

		vector<float> w_att;
		model_to_advance->Get_tunable(&w_att);

		float delta_w = 0.f, temp;
		size_t k, K = grad_att.size();
		for (k = 0; k < K; k++) {
			temp = this->Step_to_advance * grad_att[k];
			w_att[k] += temp;
			temp = abs(temp);
			if (temp > delta_w) delta_w = temp;
		}
		model_to_advance->Set_tunable(w_att);

		return delta_w;

	}

#ifdef COMPILE_BFGS
	void list_2_VectorXf(VectorXf* result, const list<float>& L) {

		*result = VectorXf(L.size());
		size_t k = 0;
		for (auto it = L.begin(); it != L.end(); it++) {
			(*result)(k) = *it;
			k++;
		}

	}

	void BFGS::__line_search(Graph_Learnable* model_to_advance, VectorXf& S, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {
 
		list<float> w_initial, w_new;
		Graph_Learnable::Weights_Manager::Get_tunable_w(&w_initial, model_to_advance);
		list<float>::iterator it_w, it_w2;
		for (it_w = w_initial.begin(); it_w != w_initial.end(); it_w++)
			w_new.push_back(1.f);
		list<size_t*>::const_iterator it_comb;

		float E_old = 0.f, E_new;
		float temp;
		for (it_comb = comb_in_train_set.begin(); it_comb != comb_in_train_set.end(); it_comb++) {
			model_to_advance->Eval_Log_Energy_function_normalized(&temp, *it_comb, comb_var);
			E_old += temp;
		}

		size_t k2;
		for (size_t k = 0; k < 5; k++) {
			S *= 0.5f;
			it_w2 = w_new.begin();
			k2 = 0;
			for (it_w = w_initial.begin(); it_w != w_initial.end(); it_w++) {
				*it_w2 = *it_w + S(k2);
				k2++;
				it_w2++;
			}
			this->Set_w(w_new, model_to_advance);

			E_new = 0.f;
			for (it_comb = comb_in_train_set.begin(); it_comb != comb_in_train_set.end(); it_comb++) {
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
		for (it_w = w_initial.begin(); it_w != w_initial.end(); it_w++) {
			*it_w2 = *it_w + S(k2);
			k2++;
			it_w2++;
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
		for (k = 0; k < (int)temp.cols(); k++)
			temp(k, k) += 1.f;

		this->pData->invB = temp * this->pData->invB * temp.transpose();
		this->pData->invB += (1.f / dot_temp) * S * S.transpose();


		//EigenSolver<MatrixXf> solver(this->pData->invB);
		//VectorXcf eig_vals = solver.eigenvalues();
		//float max_eig = abs(eig_vals(0).real()), eig_temp;
		//for (k = 1; k < (size_t)eig_vals.size(); k++) {
		//	eig_temp = abs(eig_vals(k).real());
		//	if (eig_temp > max_eig)
		//		max_eig = eig_temp;
		//}
		//return max_eig;

		float min_S = abs(S(0)), temp_S;
		for (k = 1; k < (size_t)S.size(); k++) {
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



	std::unique_ptr<I_Trainer> I_Trainer::Get_fixed_step(const float& step_size, const float& stoch_grad_percentage) {

		I_Advancer_Strategy* created = new Fixed_step(step_size);
		if (stoch_grad_percentage == 1.f) return unique_ptr<I_Trainer>(new Entire_Set(created));
		else							  return unique_ptr<I_Trainer>(new Stoch_Set(created, stoch_grad_percentage));

	}

#ifdef COMPILE_BFGS
	I_Trainer* I_Trainer::Get_BFGS(const float& stoch_grad_percentage) {

		Advancer_Concrete* created = new BFGS();
		if (stoch_grad_percentage == 1.f) return new Entire_Set(created);
		else							  return new Stoch_Set_variation(created, stoch_grad_percentage);

	}
#endif

}

