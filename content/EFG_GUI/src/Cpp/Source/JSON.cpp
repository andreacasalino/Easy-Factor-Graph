#include "../Header/JSON.h"
using namespace std;



void __append(std::string* to_modify, const std::string& to_append) {

	for (auto it = to_append.begin(); it != to_append.end(); it++)
		to_modify->push_back(*it);

}

I_JSON_composite::~I_JSON_composite() {

	for (auto it = this->Nested_elements.begin();
		it != this->Nested_elements.end(); it++)
		delete *it;

}

void I_JSON_composite::__clone_nested(I_JSON_composite* to_clone) {

	for (auto it = to_clone->Nested_elements.begin();
		it != to_clone->Nested_elements.end(); it++)
		this->Nested_elements.push_back(this->___copy(*it));

}

void JSON_tag::Add_field(const std::string& name, const std::string& value) {

	this->Nested_fields.push_back(__field());
	this->Nested_fields.back().name = name;
	this->Nested_fields.back().value = value;

}

void JSON_tag::Add_nested(const std::string& name, const I_JSON_composite& to_nest) {

	this->Nested_names.push_back(name);
	this->Nested_elements.push_back(this->___copy(&to_nest));

}

I_JSON_composite*  JSON_tag::__copy() const {

	auto __new = new JSON_tag();
	__new->__clone_nested((I_JSON_composite*)this);

	__new->Nested_names = this->Nested_names;
	__new->Nested_fields = this->Nested_fields;

	return __new;

}

void JSON_tag::__stringify(std::string* result) {

	size_t N_elements = this->Nested_elements.size() + this->Nested_fields.size(), k = 1;

	result->push_back('{');
	for (auto it = this->Nested_fields.begin();
		it != this->Nested_fields.end(); it++) {
		__append(result, "\"" + it->name + "\":\"" + it->value + "\"");

		if (k != N_elements)
			result->push_back(',');
		k++;
	}
	auto it_name = this->Nested_names.begin();
	for (auto it = this->Nested_elements.begin();
		it != this->Nested_elements.end(); it++) {
		__append(result, "\"" + *it_name + "\":");
		this->___stringify(result, *it);
		it_name++;

		if (k != N_elements)
			result->push_back(',');
		k++;
	}
	result->push_back('}');

}

void JSON_array::Append(const I_JSON_composite& to_nest) {

	this->Nested_elements.push_back(this->___copy(&to_nest));

}

I_JSON_composite*  JSON_array::__copy() const {

	auto __new = new JSON_array();
	__new->__clone_nested((I_JSON_composite*)this);
	return __new;

}

void JSON_array::__stringify(std::string* result) {

	size_t N_elements = this->Nested_elements.size(), k = 1;

	result->push_back('[');
	for (auto it = this->Nested_elements.begin();
		it != this->Nested_elements.end(); it++) {
		this->___stringify(result, *it);

		if (k != N_elements)
			result->push_back(',');
		k++;
	}
	result->push_back(']');

}

void JSON_numerical_array::__stringify(std::string* result) {

	result->push_back('[');
	size_t L = this->Values.size();
	if (L > 0) {
		if (L == 1)
			(*result) += to_string(this->Values.front());
		else {
			auto it = this->Values.begin();
			for (size_t k = 0; k < (L - 1); k++) {
				(*result) += to_string(*it);
				(*result) += ',';
				it++;
			}
			(*result) += to_string(this->Values.back());
		}
	}

	result->push_back(']');

}

JSON_numerical_array::JSON_numerical_array(const std::vector<float>& vals) {

	size_t K = vals.size();
	for (size_t k = 0; k < K; k++)
		this->Values.push_back(vals[k]);

}