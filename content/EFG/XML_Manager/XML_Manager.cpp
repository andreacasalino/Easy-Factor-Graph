/**
 * Author:    Andrea Casalino
 * Created:   03.12.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "XML_Manager.h"
#include <iostream>
#include <sstream>
using namespace std;


void XML_reader::splitta_riga(string& riga, list<string>* slices) {

	istringstream iss(riga);
	slices->clear();
	while (true) {
		if (iss.eof()) {
			break;
		}
		slices->push_back(string());
		iss >> slices->back();
	}

}

XML_reader::XML_reader(const string& name_file) {

	int line = 0;
	ifstream f(name_file);

	if (!f.is_open()) {
#ifdef __USE_THROW
		throw 0;
#else
		cout << "Warning in XML_reader::XML_reader, cannot find " << name_file << " empty XML_reader is returned " << endl;
#endif
	}

	string temp;
	list<string> slices;
	getline(f, temp); line++; //skip pramble 
	getline(f, temp); line++;
	splitta_riga(temp, &slices);


	bool parsing_validity;
	this->Tree_content = new __Tag(f, &line, slices, &parsing_validity);
	if (!parsing_validity) {
		delete this->Tree_content;
#ifdef __USE_THROW
		throw 1;
#else
		cout << "Warning: parsing error found \n";
#endif
		this->Tree_content = new __Tag("Root");
	}

	if (!f.eof()) {
#ifdef __USE_THROW
		throw 2;
#else
		cout << "Warning from XML_reader::XML_reader: multiple root tag founded, only the first is considered \n";
#endif
	}
	f.close();

}



XML_reader::Tag_readable XML_reader::Get_root() {

	Tag_readable temp(*this);
	return temp;
}

XML_reader::__Tag::__Tag(ifstream& f, int* line, std::list<std::string>& slices, bool* parsing_succeeded, __Tag* generating_father) : father(generating_father) {

	*parsing_succeeded = true;

	int line_open = *line;	
	this->line_in_file = line_open;
	if (slices.front().front() != '<') {
#ifdef __USE_THROW
		throw 0;
#else
		cout << "Error in XML_reader::XML_reader, line begin without < , line " << *line << "\n";
#endif
		*parsing_succeeded = false;
		return;
	}
	//read tag name
	bool terminator_found = false;
	if (slices.size() == 1) {
		if (slices.front().back() != '>') {
#ifdef __USE_THROW
			throw 1;
#else
			cout << "Error in XML_reader::XML_reader, tag terminator not found " << *line << "\n";
#endif
			*parsing_succeeded = false;
			return;
		}
		this->name = string(slices.front(), 1, (int)slices.front().size() - 2);
		slices.pop_front();
		slices.push_back(">");
	}
	else {
		this->name = string(slices.front(), 1);
		slices.pop_front();
		if (slices.back().size() >= (this->name.size() + 3)) {
			//chek if terminator tag is contained in last slice
			string temp(slices.back(), (int)slices.back().size() - ((int)this->name.size() + 3));
			if (temp.compare("</" + this->name + ">") == 0) {
				terminator_found = true;
				for (int k = 0; k < ((int)this->name.size() + 3); k++)
					slices.back().pop_back();
				if (slices.back().empty())
					slices.pop_back();
			}
		}
	}

	if (slices.back().back() != '>') {
#ifdef __USE_THROW
		throw 2;
#else
		cout << "Error in XML_reader::XML_reader, tag terminator not found " << *line << "\n";
#endif
		*parsing_succeeded = false;
		return;
	}
	slices.back().pop_back();
	if (slices.back().empty())
		slices.pop_back();

	//read nested words
	for (list<string>::iterator it_word = slices.begin(); it_word != slices.end(); it_word++) {
		this->Extract_word(*it_word, line, parsing_succeeded);
		if (!parsing_succeeded) {
			*parsing_succeeded = false;
			return;
		}
	}
	if (terminator_found)
		return;

	//read nested elements
	string line_new;
	list<string> slices_new;
	while (!f.eof()) {
		getline(f, line_new);
		(*line)++;
		splitta_riga(line_new, &slices_new);

		if (slices_new.front().compare("</" + this->name + ">") == 0)
			return;

		this->nested_tag.push_back(new __Tag(f, line, slices_new, parsing_succeeded, this));
		if (!parsing_succeeded) {
			*parsing_succeeded = false;
			return;
		}
	}

#ifdef __USE_THROW
	throw 3;
#else
	cout << "Error in XML_reader::XML_reader, tag open at line " << line_open << " not properly closed \n";
#endif
	*parsing_succeeded = false;

}

void XML_reader::__Tag::Extract_word(string& raw, int* line, bool* parsing_succeeded) {

	this->fields.push_back(Field());
	int pos_equal = 0;
	for (int k = 0; k < raw.size(); k++) {
		if (raw[k] == '=') {
			pos_equal = k;
			break;
		}
	}

	if (pos_equal == 0) {
#ifdef __USE_THROW
		throw 0;
#else
		cout << "Error in XML_reader::Tag::Extract_word, invalid word at line " << *line << endl;
#endif
		*parsing_succeeded = false;
		return;
	}

	this->fields.back().name = string(raw, 0, pos_equal);
	string temp = string(raw, pos_equal + 1);

	if ((temp.front() != '\"') || (temp.back() != '\"')) {
#ifdef __USE_THROW
		throw 1;
#else
		cout << "Error in XML_reader::Tag::Extract_word, word not delimited by \" at line " << *line << endl;
#endif
		*parsing_succeeded = false;
		return;
	}

	this->fields.back().content = string(temp, 1, temp.size() - 2);

}

XML_reader::__Tag::~__Tag() {

	for (list<__Tag*>::iterator it_t = this->nested_tag.begin(); it_t != this->nested_tag.end(); it_t++)
		delete *it_t;

}

void XML_reader::Reprint(const std::string& file_name) {

	ofstream f(file_name);
	if (!f.is_open()) {
		cout << " reprinting file not found\n";
		f.close();
		return;
	}

	this->Reprint(f);
	f.close();

}

void XML_reader::Reprint(std::ostream& stream_to_use) {

	stream_to_use << "<?xml version=\"1.0\"?>\n";
	this->Tree_content->Reprint(stream_to_use, "", true);

}

void XML_reader::__Tag::Reprint(std::ostream& stream_to_use, const std::string& space_to_use, const bool& is_the_root) {

	stream_to_use << space_to_use << "<" << this->name << "";
	for (auto it = this->fields.begin(); it != this->fields.end(); it++) {
		stream_to_use << " " << it->name << "=\"" << it->content << "\"";
	}
	stream_to_use << ">";

	if (!this->nested_tag.empty()) {
		stream_to_use << endl;
		for (auto it = this->nested_tag.begin(); it != this->nested_tag.end(); it++)
			(*it)->Reprint(stream_to_use, space_to_use + "  ", false);
		stream_to_use << space_to_use;
	}

	stream_to_use << "</" << this->name << ">";
	if (!is_the_root)
		stream_to_use << endl;

}







XML_reader::Tag_readable::Tag_readable(XML_reader& reader) {

	this->encapsulated = reader.Tree_content;

}

bool XML_reader::Tag_readable::Exist_Nested_tag(const string& name_nested) {

	for (auto it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) return true;
	}
	return false;

}

void XML_reader::Tag_readable::Get_Nested(const string& name_nested, list<Tag_readable>* nested) {

	nested->clear();
	for (list<__Tag*>::iterator it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) nested->push_back(Tag_readable(*it));
	}

}

list<XML_reader::Tag_readable>	XML_reader::Tag_readable::Get_Nested(const string& name_nested) {

	list<Tag_readable> temp;
	this->Get_Nested(name_nested, &temp);
	return temp;

}

XML_reader::Tag_readable XML_reader::Tag_readable::Get_Nested_first_found(const string& name_nested) {

	XML_reader::Tag_readable res;
	for (list<__Tag*>::iterator it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) {
			res.encapsulated = *it;
			return res;
		}
	}
#ifdef __USE_THROW
	throw 0;
#else
	cout << "not able to find nested tag " << name_nested << " in tag " << this->encapsulated->name << " at line " << this->encapsulated->line_in_file << endl;
#endif
	return Tag_readable();

}

XML_reader::Tag_readable XML_reader::Tag_readable::Get_Nested(const list<string>& path) {

	__Tag* cursor = this->encapsulated;
	for (auto it = path.begin(); it != path.end(); it++) {
		Tag_readable attual(cursor);
		attual = attual.Get_Nested_first_found(*it);
		cursor = attual.encapsulated;
	}

	return Tag_readable(cursor);

}

void XML_reader::Tag_readable::Get_all_nested(std::list<Tag_readable>* nested_tags) {

	nested_tags->clear();
	for (auto it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++)
		nested_tags->push_back(Tag_readable(*it));

}




std::list<std::string>	XML_reader::Tag_readable::Get_attribute_names() {

	list<string> names;
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++)
		names.push_back(it->name);
	return names;

}

bool XML_reader::Tag_readable::Exist_Field(const string& name_field) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(name_field) == 0) return true;
	}
	return false;

}

const string* XML_reader::Tag_readable::Get_Attribute_first_found(const string& name) {

	for (list<__Tag::Field>::iterator it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(name) == 0) {
			return &it->content;
		}
	}
#ifdef __USE_THROW
	throw 0;
#else
	cout << "not able to find field " << name << " in tag " << this->encapsulated->name << " at line " << this->encapsulated->line_in_file << endl;
#endif
	return NULL;

}

void  XML_reader::Tag_readable::Get_all_Attributes(list<string>* field_names, list<string>* fields) {
	
	field_names->clear();
	fields->clear();
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		field_names->push_back(it->name);
		fields->push_back(it->content);
	}

}

list<string>  XML_reader::Tag_readable::Get_Attributes(const string& field_name) {

	list<string> vals;
	this->Get_Attributes(field_name, &vals);
	return vals;

}

void  XML_reader::Tag_readable::Get_Attributes(const string& field_name, list<string>* results) {

	results->clear();
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(field_name) == 0) {
			results->push_back(it->content);
		}
	}

}






void XML_reader::Tag_readable::Set_attribute_name(const std::string& field_name, const std::string& new_name) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(field_name) == 0)
			it->name = new_name;
	}

}

void XML_reader::Tag_readable::Set_attribute_name(const std::string& name_attribute, const std::string& val_attribute, const std::string& new_name_attribute) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if ((it->name.compare(name_attribute) == 0) && (it->content.compare(val_attribute) == 0))
			it->name = new_name_attribute;
	}

}

void XML_reader::Tag_readable::Set_attribute_value(const std::string& field_name, const std::list<std::string>& new_vals) {

	list<string*> val_to_change;
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(field_name) == 0) 
			val_to_change.push_back(&it->content);
	}

	if (val_to_change.size() != new_vals.size()) {
		cout << " wrong number of values, ignored\n";
		return;
	}

	auto it = new_vals.begin();
	for (auto it_v = val_to_change.begin(); it_v != val_to_change.end(); it_v++) {
		**it_v = *it;
		it++;
	}

}

void XML_reader::Tag_readable::Set_attribute_value(const std::string& name_attribute, const std::string& val_attribute, const std::string& new_value) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if ((it->name.compare(name_attribute) == 0) && (it->content.compare(val_attribute) == 0))
			it->content = new_value;
	}

}

void XML_reader::Tag_readable::Remove_Attribute(const std::string& field_name, const std::string& value) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if ((it->name.compare(field_name) == 0) && (it->content.compare(value) == 0)) {
			it = this->encapsulated->fields.erase(it);
			return;
		}
	}

}

void XML_reader::Tag_readable::Remove_Attribute(const std::string& field_name) {

	auto it = this->encapsulated->fields.begin();
	while (it != this->encapsulated->fields.end()) {
		if (it->name.compare(field_name) == 0)
			it = this->encapsulated->fields.erase(it);
		else
			it++;
	}

}

void XML_reader::Tag_readable::Add_Attribute(const std::string& field_name, const std::string& content) {

	this->encapsulated->fields.push_back(__Tag::Field());
	this->encapsulated->fields.back().name = field_name;
	this->encapsulated->fields.back().content = content;

}

XML_reader::Tag_readable XML_reader::Tag_readable::Add_Nested_and_return_created(const std::string& tag_name) {

	__Tag* created = new __Tag(tag_name);
	this->encapsulated->nested_tag.push_back(created);
	return Tag_readable(created);

}

void XML_reader::Tag_readable::Add_Nested(const std::string& tag_name) {

	this->Add_Nested_and_return_created(tag_name);

}

void XML_reader::Tag_readable::Remove() {

	if (this->encapsulated->father == NULL) {
		cout << "You cannot remove the root, ignored\n";
		return;
	}

	for (auto it = this->encapsulated->father->nested_tag.begin();
		it != this->encapsulated->father->nested_tag.end(); it++) {
		if (*it == this->encapsulated) {
			it = this->encapsulated->father->nested_tag.erase(it);
			break;
		}
	}

	delete this->encapsulated;

	this->encapsulated = NULL;

}