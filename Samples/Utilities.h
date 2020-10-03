#ifndef   _UTILITIES___H_
#define   _UTILITIES___H_

#include <string>
#include <list>
#include <vector>
#include <distribution/Distribution.h>
#include <cmath>
#include <fstream>
using namespace std;



ostream& operator<<(ostream& s, const EFG::distr::DiscreteDistribution& pot) {

	size_t K = pot.GetVariables().size(), k;

	auto it = pot.getIter();
	if (it.isNotAtEnd()) {
		s << "<" << pot.GetVariables()[0]->GetName();
		for (k = 1; k < K; ++k) s << "," << pot.GetVariables()[k]->GetName();
		s << ">\n";
		EFG::itr::forEach<EFG::distr::DiscreteDistribution::constIterator>(it, [&s, &k, &K](EFG::distr::DiscreteDistribution::constIterator& itt) {
			s << "<" << itt->GetIndeces()[0];
			for (k = 1; k < K; ++k) s << "," << itt->GetIndeces()[k];
			s << "> -> ";
			s << itt->GetVal() << "\n";
		});
	}

	return s;

}

//print a list of elements
template<typename Collection>
void print_distribution(const Collection& distr) {

	for (auto it = distr.begin(); it != distr.end(); ++it) {
		cout << " " << *it;
	}
	cout << endl;

}

//normalize the values such that their sum is equal to 1
void normalize(vector<float>* to_normalize) {

	float Z = 0.f;
	auto it = to_normalize->begin();
	for (it; it != to_normalize->end(); ++it)
		Z += *it;

	Z = 1.f / Z;
	for (it = to_normalize->begin(); it != to_normalize->end(); ++it)
		*it *=  Z;

}

//compute the empirical frequencies of a variable (var_sampled), considering a list of realizations (sample) taken as samples from a joint probability distribution (the one involving vars_in_sample) 
//the result is put into marginals
void Get_empirical_frequencies(vector<float>* marginals, const list<vector<size_t>>& sample, EFG::CategoricVariable* var_desired, const vector<EFG::CategoricVariable*>& vars_in_sample ) {

	if (sample.front().size() != vars_in_sample.size())
		throw std::runtime_error("invalid inputs");

	marginals->clear();

	size_t var_pos, k=0;
	bool managed = false;
	for (auto it = vars_in_sample.begin(); it != vars_in_sample.end(); ++it) {
		if (*it == var_desired) {
			var_pos = k;
			managed = true;
			break;
		}
		++k;
	}
	if (!managed)  throw std::runtime_error("variable not found");

	float temp;
	size_t var_size = var_desired->size();
	list<vector<size_t>>::const_iterator it_sample;
	marginals->reserve(var_size);
	for (size_t k = 0; k < var_size; ++k) {
		temp = 0.f;

		for (it_sample = sample.begin(); it_sample != sample.end(); ++it_sample) {
			if ((*it_sample)[var_pos] == k)
				temp += 1.f;
		}


		temp = temp / (float)sample.size();
		marginals->push_back(temp);
	}

}

//similar to the above function, computing in single call the empirical distributions of all the variables in vars_to_search
float Get_empirical_frequencies(const list<vector<size_t>>& sample, const vector<size_t>& combination_to_search, const vector<EFG::CategoricVariable*>& vars_to_search, const vector<EFG::CategoricVariable*>& vars_in_sample) {

	if (sample.front().size() != vars_in_sample.size())
		throw std::runtime_error("invalid inputs");
	if (combination_to_search.size() != vars_to_search.size())
		throw std::runtime_error("invalid inputs");

	vector<size_t> positions;
	positions.reserve(vars_to_search.size());
	size_t p;
	bool found;
	size_t v, V = vars_in_sample.size();
	for (auto itv = vars_to_search.begin(); itv != vars_to_search.end(); ++itv) {
		p = 0;
		found = false;
		for (v = 0; v < V; ++v) {
			if (vars_to_search[v] == *itv) {
				positions.push_back(p);
				found = true;
				break;
			}
			++p;
		}
		if (!found) throw std::runtime_error("variable not found");
	}


	float freq = 0;
	bool match;
	list<size_t>::const_iterator it1, it2;
	list<size_t>::iterator it_pos;
	size_t c, C = combination_to_search.size();
	for (auto it = sample.begin(); it != sample.end(); ++it) {
		match = true;

		for (c = 0; c < C; ++c) {
			if (combination_to_search[c] != (*it)[positions[c]]) {
				match = false;
				break;
			}
		}

		if (match) freq += 1.f;
	}

	return freq / (float)sample.size();

}

//print into a file some samples taken from the joint probability distribution that involves vars.
//Such a file can be then exploited for training a graphical model
void Print_set_as_training_set(const string& file_name, const vector<EFG::CategoricVariable*>& vars, const list<vector<size_t>>& samples) {

	ofstream f(file_name);
	if (!f.is_open()) throw std::runtime_error("invalid file for exporting samples");

	for (auto it = vars.begin(); it != vars.end(); ++it)
		f << " " << (*it)->GetName();
	f << endl;

	auto c = samples.front().begin();
	auto r_end = samples.end(); r_end--;
	for (auto r = samples.begin(); r != samples.end(); ++r) {
		for (auto c = r->begin(); c != r->end(); ++c)
			f << " " << *c;

		if (r != r_end) f << endl;
	}

	f.close();

};

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
std::string Get_prefix() {

		std::string pref;

#ifdef _MSC_VER
		if (IsDebuggerPresent()) {
				//launched from inside VS: app folder is the one containing the project
				pref = "./";
		}
		else {
				//launched from outside VS: app folder is the one containing the .exe (which changes accirding to Debug, Release, etc... )
				pref = "../";
		}
#else
		pref = "./";
#endif

		return pref;

}

#endif