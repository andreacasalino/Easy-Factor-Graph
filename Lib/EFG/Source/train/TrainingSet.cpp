/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <train/TrainingSet.h>
#include <Parser.h>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <Error.h>
using namespace std;

namespace EFG::train {
	

	TrainingSet::TrainingSet(const std::string& file_to_import) {

		ifstream f_set(file_to_import);
		if (!f_set.is_open()) {
			f_set.close();
			throw Error("train::TrainingSet" , "invalid training set file");
		}

		string line;
		list<string> slices;

		if (f_set.eof()) {
			f_set.close();
			throw Error("train::TrainingSet" , "invalid training set file"); //found empty training set
		}

		getline(f_set, line);
		slices =  xmlPrs::Parser::sliceFragments(line);
		vector<string> names;
		names.reserve(slices.size());
		while (!slices.empty()) {
			names.push_back(slices.front());
			slices.pop_front();
		}

		if (f_set.eof()) {
			f_set.close();
			throw Error("train::TrainingSet" , "invalid training set file"); //found empty training set
		}

		std::list<std::pair<size_t* , size_t>> raw;
		size_t k, K;
		while (!f_set.eof()) {
			getline(f_set, line);
			slices = xmlPrs::Parser::sliceFragments(line);

			K = slices.size();
			raw.emplace_back(make_pair((size_t*)malloc(sizeof(size_t) * K) , K));
			for(k=0; k<K; ++k){
				raw.back().first[k] = (size_t)atoi(slices.front().c_str());
				slices.pop_front();
			} 
		}

		f_set.close();

		this->__Import(names , raw);

	}

	void TrainingSet::__Import(const std::vector<std::string>& var_names, const std::list<std::pair<size_t* , size_t>>& combs_raw) {

		if (combs_raw.empty()) throw Error("train::TrainingSet" , "train set empty is invalid");
		if (var_names.empty()) throw Error("train::TrainingSet" , "train set empty is invalid");

		this->varNames = var_names;

		size_t K = this->varNames.size(), k;
		this->maxCombVals.reserve(K);
		for (k = 0; k < K; ++k) this->maxCombVals.push_back(0);
		std::for_each(combs_raw.begin(), combs_raw.end(), [this, &K, &k](const std::pair<size_t*, size_t>& p) {
			if (p.second != K) throw Error("train::TrainingSet" , "mismatch in the combination sizes");
			this->combs.push_back(p.first);
			for (k = 0; k < K; ++k) {
				if (p.first[k] > this->maxCombVals[k])
					this->maxCombVals[k] = p.first[k];
			}
		});
		
	}

	void TrainingSet::operator+=(const TrainingSet& tr){

		if(tr.varNames.size() != this->varNames.size()) throw Error("train::TrainingSet" , "cannot copy training set with different combinations sizes");

		size_t k, K = this->varNames.size();
		std::for_each(tr.combs.begin(), tr.combs.end(), [this, &K, &k](size_t* c) {
			this->combs.push_back((size_t*)malloc(K * sizeof(size_t)));
			for (k = 0; k < K; ++k) {
				this->combs.back()[k] = c[k];
				if (this->combs.back()[k] > this->maxCombVals[k])
					this->maxCombVals[k] = this->combs.back()[k];
			}
		});

	}

	void TrainingSet::Print(const std::string& file_name) const {

		ofstream f(file_name);
		if (!f.is_open()) {
			f.close();
			throw Error("train::TrainingSet" , "invalid file to print training set");
		}

		size_t K = this->varNames.size();
		f << this->varNames[0];
		size_t k; 
		for (k = 1; k < K; ++k) {
			f << " " << this->varNames[k];
		}
		std::for_each(this->combs.begin(), this->combs.end(), [&f, &K, &k](size_t* c) {
			f << endl;
			f << c[0];
			for (k = 1; k < K; ++k) f << " " << c[k];
		});

		f.close();

	}

	vector<CategoricVariable*> extract(const std::vector<string>& names, const std::vector<size_t>& max_vals, const model::GraphLearnable& model_to_train) {
		auto vars = model_to_train.GetAllVariables();
		if(names.size() != vars.size()) throw Error("train::TrainingSet" , "invalid training set sizes");

		vector<CategoricVariable*> result;
		result.reserve(names.size());
		for (size_t k = 0; k < names.size(); ++k) {
			auto temp = model_to_train.FindVariable(names[k]);
			if(temp == nullptr)  throw Error("train::TrainingSet" , "variable not found in the model");
			result.push_back(temp);
			if(max_vals[k] >= temp->size()) throw Error("train::TrainingSet" , "invalid combinations in training set");
		}
		return result;
	}
	TrainingSet::UsableTrainingSet::UsableTrainingSet(TrainingSet& set, const model::GraphLearnable& model_to_train, const float& Sample_percentage) :
	distr::Combinations(extract(set.varNames , set.maxCombVals, model_to_train)) , source(&set) , originalSize(set.combs.size()) , Observer(set){

		if(Sample_percentage == 1.f){
			for(auto it=set.combs.begin(); it!=set.combs.end(); ++it) this->combinations.push_back(*it);
		}
		else{
			this->originalSize = (size_t)ceilf(set.combs.size() * Sample_percentage);
			this->Resample();
		}

	}

	void TrainingSet::UsableTrainingSet::Resample(){

		if(this->originalSize >= this->source->combs.size()) return;

		size_t k;
		if(this->combinations.size() >= this->originalSize){
			for(k = 0; k<this->originalSize; ++k) this->combinations.pop_front();
		}

		list<size_t*>::iterator sample;
		for(k = 0; k<this->originalSize; ++k){
			sample = this->source->combs.begin();
			advance(sample, rand() % this->source->combs.size());
			this->combinations.push_front(*sample);
		}

	}

	TrainingSet::UsableTrainingSet::~UsableTrainingSet(){

		for(size_t k=0; k<this->originalSize; ++k) this->combinations.pop_front();

	}

}

