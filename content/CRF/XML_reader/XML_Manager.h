// Author: Andrea Casalino
// mail: andrecasa91@gmail.com

#pragma once

#ifndef _XML_IMPORTER_H_
#define _XML_IMPORTER_H_

#include <list>
#include <string>
#include <fstream>

void splitta_riga(std::string& riga, std::list<std::string>* slices);

class XML_reader
{
public:
	class Tag_readable;

	class Tag {
		friend class XML_reader::Tag_readable;
	public:
		struct Field {
			std::string name;
			std::string content; 
		};
	// constructor
		Tag(std::ifstream& f, int* line, std::list<std::string>& field_slices, Tag* generating_father = NULL);
		Tag(const std::string& name_to_use) : name(name_to_use), line_in_file(-1) {};
		~Tag();

		void Reprint(std::ostream& stream_to_use, const std::string& space_to_use, const bool& is_the_root);
	protected:
		void Extract_word(std::string& raw, int* line);
	// data
		Tag*                father;
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

	//////////////
	//nested Tag//
	//////////////
		//check whether a certain tag is present
		bool											Exist_Nested_tag(const std::string& name_nested); 
		//all nested with that name are returned
		void											Get_Nested(const std::string& name_nested, std::list<Tag_readable>* nested); 
		std::list<Tag_readable>							Get_Nested_fast(const std::string& name_nested);
		//return first element found with that name
		Tag_readable									Get_Nested(const std::string& name_nested);
		//path is the path of the XML tree starting from this tag
		Tag_readable									Get_Nested(std::list<std::string> path); 

	////////////////////////
	//Fields (=Attributes)//
	////////////////////////
		//check whether a certain field is present
		bool											Exist_Field(const std::string& name_field); 
		//return the value related to a particular field
		std::string										Get_value(const std::string& name); 
		//return the values related to the specified fields  
		std::list<std::string>							Get_values(std::list<std::string>& field_names); 
		void											Get_values(std::list<std::string>& field_names, std::list<std::string>* fields);
		//return the values related to all the nested fields and the name of the fields
		void											Get_all_values(std::list<std::string>* field_names, std::list<std::string>* fields); 
		std::list<std::string>							Get_values_specific_field_name(const std::string& field_name);
		void											Get_values_specific_field_name(const std::string& field_name, std::list<std::string>* results);

	/////////////
	// Setters //
	/////////////
		void											Set_name(const std::string& new_name) { this->encapsulated->name = new_name; };
		// change the name of  every field with the passed name
		void											Set_field_name(const std::string& field_name, const std::string& new_names); 
		// values must be in the same number of the field with that name
		void											Set_field_content(const std::string& field_name, const std::list<std::string>& new_vals);
		void											Remove();
		// remove the field with name field_name and content equalt o value
		void											Remove_field(const std::string& field_name, const std::string& value); 
		// remove all the fields with that name
		void											Remove_field(const std::string& field_name); 

		void											Add_field(const std::string& field_name, const std::string& content);
		void											Add_Nested(const std::string& tag_name); //initially is built empty
		Tag_readable									Add_Nested_and_return_created(const std::string& tag_name);

	private:
		Tag* encapsulated;
	};



// constructor
	XML_reader(std::string source_file);
	XML_reader() { this->Tree_content = new Tag("Root"); };
	~XML_reader() { delete this->Tree_content; };

// methods
	Tag_readable Get_root();
	void		 Reprint(const std::string& file_name);
	void		 Reprint(std::ostream& stream_to_use);
private:
// data
	Tag* Tree_content;	//only the root is allocated
};


#endif