//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once

#ifndef _XML_IMPORTER_H_
#define _XML_IMPORTER_H_

#include <list>
#include <fstream>
#include <string>

class XML_reader
{
public:
// constructor
	XML_reader(std::string source_file);
	~XML_reader() { delete this->Tree_content; };
// methods
	static void splitta_riga(std::string& riga, std::list<std::string>* slices);
// getters
	class Tag_readable;
	class Tag {
		friend class XML_reader::Tag_readable;
	public:
		struct Field {
			std::string name;
			std::string content; };
		// constructor
		Tag(std::ifstream& f);
		~Tag();
	protected:
		// methods
		void Extract_word(std::string& raw);
		// data
		int					line_in_file;
		std::string			name;
		std::list<Field>	fields;
		std::list<Tag*>		nested_tag;
	};
	class Tag_readable {
	public:
	// constructor
		Tag_readable(Tag* enc) : encapsulated(enc) {};
		Tag_readable() : encapsulated(NULL) {};
	// methods

		//////////////
		//nested Tag//
		//////////////
		bool					Exist_Nested_tag(const std::string& name_nested); //check whether a certain tag is present
		void					Get_Nested(const std::string& name_nested, std::list<Tag_readable>* nested); //all nested with that name are returned
		std::list<Tag_readable>	Get_Nested_fast(const std::string& name_nested);
		Tag_readable			Get_Nested(const std::string& name_nested); //return first element found with that name
		Tag_readable			Get_Nested(std::list<std::string> path); //path is the path of the XML tree starting from this tag

		////////////////////////
		//Fields (=Attributes)//
		////////////////////////
		bool					Exist_Field(const std::string& name_field); //check whether a certain field is present
		std::string			    Get_value(const std::string& name); //return the value related to a particular field
		std::list<std::string>  Get_values(std::list<std::string>& field_names); //return the values related to the specified fields  
		void  Get_values(std::list<std::string>& field_names, std::list<std::string>* fields);
		void  Get_all_values(std::list<std::string>* field_names, std::list<std::string>* fields); //return the values related to all the nested fields and the name of the fields
		std::list<std::string>  Get_values_specific_field_name(const std::string& field_name);
		void  Get_values_specific_field_name(const std::string& field_name, std::list<std::string>* results);

	private:
		Tag* encapsulated;
	};
	Tag_readable Get_root();
private:
// data
	static int line;
	Tag* Tree_content; //only the root is allocated
};


#endif