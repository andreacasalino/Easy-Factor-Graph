#pragma once
#ifndef   _UTILITIES___H_
#define   _UTILITIES___H_

#include <string>
#include <list>
#include "../EFG/Header/Potential.h"
#include <cmath>
using namespace std;

//print a list of elements
template<typename T>
void print_distribution(const list<T>& distr) {

	for (auto it = distr.begin(); it != distr.end(); it++) {
		cout << " " << *it;
	}
	cout << endl;

}

//normalize the values such that their sum is equal to 1
void normalize(list<float>* to_normalize) {

	float Z = 0.f;
	auto it = to_normalize->begin();
	for (it; it != to_normalize->end(); it++)
		Z += *it;

	Z = 1.f / Z;
	for (it = to_normalize->begin(); it != to_normalize->end(); it++)
		*it *=  Z;

}

//compute the empirical frequencies of a variable (var_sampled), considering a list of realizations (sample) taken as samples from a joint probability distribution (the one involving vars_in_sample) 
//the result is put into marginals
void Get_empirical_frequencies(list<float>* marginals, const list<list<size_t>>& sample, EFG::Categoric_var* var_desired, const list<EFG::Categoric_var*>& vars_in_sample ) {

	marginals->clear();

	size_t var_pos, k=0;
	bool managed = false;
	for (auto it = vars_in_sample.begin(); it != vars_in_sample.end(); it++) {
		if (*it == var_desired) {
			var_pos = k;
			managed = true;
			break;
		}
		k++;
	}
	if (!managed)  throw 0; // varaible not found

	float temp;
	size_t var_size = var_desired->size();
	list<list<size_t>>::const_iterator it_sample;
	list<size_t>::const_iterator it_temp;
	for (size_t k = 0; k < var_size; k++) {
		temp = 0.f;

		for (it_sample = sample.begin(); it_sample != sample.end(); it_sample++) {
			it_temp = it_sample->begin();
			advance(it_temp , var_pos);

			if (*it_temp == k)
				temp += 1.f;
		}


		temp = temp / (float)sample.size();
		marginals->push_back(temp);
	}

}

//similar to the above function, computing in single call the empirical distributions of all the variables in vars_to_search
float Get_empirical_frequencies(const list<list<size_t>>& sample, const list<size_t>& combination_to_search, const list<EFG::Categoric_var*>& vars_to_search, const list<EFG::Categoric_var*>& vars_in_sample) {

	if (sample.front().size() != vars_in_sample.size())
		abort();
	if (combination_to_search.size() != vars_to_search.size())
		abort();

	list<size_t> positions;
	list<EFG::Categoric_var*>::const_iterator itv_2;
	size_t p;
	bool found;
	for (auto itv = vars_to_search.begin(); itv != vars_to_search.end(); itv++) {
		p = 0;
		found = false;
		for (itv_2 = vars_in_sample.begin(); itv_2 != vars_in_sample.end(); itv_2++) {
			if (*itv_2 == *itv) {
				positions.push_back(p);
				found = true;
				break;
			}
			p++;
		}
		if (!found)
			abort();
	}


	float freq = 0;
	bool match;
	list<size_t>::const_iterator it1, it2;
	list<size_t>::iterator it_pos;
	for (auto it = sample.begin(); it != sample.end(); it++) {
		match = true;

		it1 = combination_to_search.begin();
		for (it_pos = positions.begin(); it_pos != positions.end(); it_pos++) {
			it2 = it->begin();
			advance(it2, *it_pos);
			if (*it2 != *it1) {
				match = false;
				break;
			}
			it1++;
		}

		if (match) freq += 1.f;
	}

	return freq / (float)sample.size();

}

//print into a file some samples taken from the joint probability distribution that involves vars.
//Such a file can be then exploited for training a graphical model
void Print_set_as_training_set(const string& file_name, const list<EFG::Categoric_var*>& vars, const list<list<size_t>>& samples) {

	ofstream f(file_name);
	if (!f.is_open()) throw 0; //invalid file for exporting samples

	for (auto it = vars.begin(); it != vars.end(); it++)
		f << " " << (*it)->Get_name();
	f << endl;

	auto c = samples.front().begin();
	auto r_end = samples.end(); r_end--;
	for (auto r = samples.begin(); r != samples.end(); r++) {
		for (auto c = r->begin(); c != r->end(); c++)
			f << " " << *c;

		if (r != r_end) f << endl;
	}

	f.close();

};

#endif