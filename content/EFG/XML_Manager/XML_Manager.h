/**
 * Author:    Andrea Casalino
 * Created:   03.12.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#ifndef _XML_IMPORTER_H_
#define _XML_IMPORTER_H_

#include <list>
#include <string>
#include <fstream>
#define __USE_THROW

/** \brief when you create a define named __USE_THROW, errors are raised when inconsistent actions are taken, 
as for example pass an inexistent file to the constrcutor of the reader. Otherwise, instructions are simply ignored.
* /*/
class XML_reader
{
public:
	class Tag_readable;
private:
	class __Tag {
		friend class XML_reader::Tag_readable;
	public:
		struct Field {
			std::string name;
			std::string content;
		};
		// constructor
		__Tag(std::ifstream& f, int* line, std::list<std::string>& field_slices, bool* parsing_succeeded, __Tag* generating_father = NULL);
		__Tag(const std::string& name_to_use) : name(name_to_use), line_in_file(-1) {};
		~__Tag();

		void Reprint(std::ostream& stream_to_use, const std::string& space_to_use, const bool& is_the_root);
	protected:
		void Extract_word(std::string& raw, int* line, bool* parsing_succeeded);
	// data
		__Tag*						father;
		int								line_in_file;
		std::string					name;
		std::list<Field>			fields;
		std::list<__Tag*>		nested_tag;
	};
public:

	/** \brief The info pertaining to a particular tag are wrapped in this class.
	* /*/
	class Tag_readable {
	public:

		/** \brief The wrapped tag is the root of the passed structure.
		\details The copy of a Tag_readable is pretty fast, since only a pointer to the data is wrapped.
		@param[in] structure the structure whose root must be wrapped
		* /*/
		Tag_readable(XML_reader& structure); 

	//////////////////////////////////////
	// Methods handling nested Tags //
	//////////////////////////////////////

		/** \brief Getter for the name of the tag
		@param[out] return the name of the wrapped tag
		* /*/
		const std::string&					Get_tag_name() const { return this->encapsulated->name; };

		/** \brief Returns true in case it exists a tag whose name is the passed one.
		@param[in] name_nested the name of the tag whose existance must be checked
		* /*/
		bool											Exist_Nested_tag(const std::string& name_nested); 

		/** \brief Get all the nested tags with a specific name.
		@param[in] name_nested the name of the tags to return
		@param[out] nested the list of tags whose name matches with name_nested
		* /*/
		void											Get_Nested(const std::string& name_nested, std::list<Tag_readable>* nested); 

		/** \brief Similar to Tag_readable::Get_Nested(const std::string& name_nested, std::list<Tag_readable>* nested).
		\details Here the list of nested tag is returned as a copy.
		@param[in] name_nested the name of the tags to return
		@param[out] return the list of tags whose name matches with name_nested
		* /*/
		std::list<Tag_readable>		Get_Nested(const std::string& name_nested);

		/** \brief Similar to Tag_readable::Get_Nested(const std::string& name_nested).
		\details Here only a single tag is returned: the first found with the passed name.
		In case that it does not exists a tag in the specified position, an invalid Tag_readable is returned.
		@param[in] name_nested the name of the tags to return
		@param[out] return the tag whose name matches with name_nested
		* /*/
		Tag_readable							Get_Nested_first_found(const std::string& name_nested);

		/** \brief Get a tag in a specified position in the structure.
		\details The term Position refers to a chain of tag names. Essentially, the path is
		the series of tag to traverse to get a certain nested tag.
		Consider this example:
		<A><B><C><D></D></C></B></A>.
		The path of tag D from A is: {B,C,D}. At the same time, the path of tag D from
		B is: {C,D}. Therefore, If tou wanna get D using Tag_readable::Get_Nested(const std::list<std::string>& position) on Tag wrapping A,
		you should pass as input {B,C,D}. Instead, when you want to get D from a Tag_readable wrapping B, you should pass as input {C,D}.
		In case that it does not exists a tag in the specified position, an invalid Tag_readable is returned.
		@param[in] position is the sequence of tag to traverse to get the one of interest from the one wrapped in the calling object
		@param[out] return the tag in the specified position
		* /*/
		Tag_readable							Get_Nested(const std::list<std::string>& position); 

		/** \brief Get all the nested tags, no matter their names.
		@param[out] nested_tags the list of nested tag
		* /*/
		void											Get_all_nested(std::list<Tag_readable>* nested_tags);

	///////////////////////////////////
	// Methods handling attributes //
	///////////////////////////////////

		/** \brief Gets the names of the all the attributes in this tag
		@param[out] return the attributes names
		* /*/
		std::list<std::string>				Get_attribute_names();

		/** \brief Returns true in case it exists an attribute in this tag whose name is the passed one.
		@param[in] name_field the name of the attribute whose existance must be checked
		* /*/
		bool											Exist_Field(const std::string& name_attribute); 

		/** \brief Get the values of the attributes with the specific name.
		@param[in] name_attribute the name of the attributes whose values must be returned
		@param[out] results the list of attribute values
		* /*/
		void											Get_Attributes(const std::string& name_attribute, std::list<std::string>* results);

		/** \brief Similar to Tag_readable::Get_Attributes(const std::string& name_attribute, std::list<std::string>* results).
		\details Here the list of attributes is returned as a copy.
		@param[in] name_attribute the name of the attributes whose values must be returned
		@param[out] return the list of attribute values
		* /*/
		std::list<std::string>				Get_Attributes(const std::string& name_attribute);

		/** \brief Similar to Tag_readable::Get_Attributes(const std::string& name_attribute).
		\details Here only a single attribute value is returned: the first found with the passed name.
		In case that it does not exists an attribute with the specified position, NULL is returned.
		@param[in] name_attribute the name of the attribute whose value must be returned
		@param[out] return the attributes's value
		* /*/
		const std::string*						Get_Attribute_first_found(const std::string& name_attribute);

		/** \brief Get all the attributes in this tag, together with the corresponding names.
		@param[out] names_attribute The names of all the attributes in this tag
		@param[out] values_attributes The values that correspond to the returned attributes names
		* /*/
		void											Get_all_Attributes(std::list<std::string>* names_attribute, std::list<std::string>* values_attributes); 

	///////////////
	// Modifiers //
	///////////////

		/** \brief Change the name of this tag.
		@param[in] new_name the new name to consider
		* /*/
		void											Set_tag_name(const std::string& new_name) { this->encapsulated->name = new_name; };

		/** \brief Change the name of the specified attribute.
		\details In case the name passed as input does not exists, this instruction is ignored.
		In case of multiple attributes with this name, they are all changed.
		@param[in] name_attribute the attribute whose name is to change
		@param[in] new_name_attribute the new name to consider for the specified attribute
		* /*/ 
		void											Set_attribute_name(const std::string& name_attribute, const std::string& new_name_attribute);

		/** \brief Similar to XML_Manager::Set_attribute_name(const std::string& name_attribute, const std::string& new_name_attribute).
		\details Here only a specific attribute is changed: the one with the passed name and value. In case such an attribute does not existst,
		this instruction is ignored.
		@param[in] name_attribute the name of the attributes whose name have to be changed
		@param[in] val_attribute the value of the attribute of interest
		@param[in] new_name_attribute the new name to impose for the attribute
		* /*/
		void											Set_attribute_name(const std::string& name_attribute, const std::string& val_attribute, const std::string& new_name_attribute);

		/** \brief Change the values of all the attributes with the passed name.
		\details In case the number of attributes with passed name is different from the size od the passed list,
		this instruction is ignored.
		@param[in] name_attribute the name of the attributes whose name have to be changed
		@param[in] new_values the new values to consider for all the attributes having the passed name
		* /*/ 
		void											Set_attribute_value(const std::string& name_attribute, const std::list<std::string>& new_values);

		/** \brief Similar to XML_Manager::Set_attribute_value(const std::string& name_attribute, const std::list<std::string>& new_values).
		\details Here only a specific attribute is changed: the one with the passed name and value. In case such an attribute does not existst,
		this instruction is ignored.
		@param[in] name_attribute the name of the attributes whose name have to be changed
		@param[in] val_attribute the value of the attribute of interest
		@param[in] new_value the new value to consider for all the attributes having the passed name
		* /*/
		void											Set_attribute_value(const std::string& name_attribute, const std::string& val_attribute, const std::string& new_value);

		/** \brief Remove this tag from the structure
		* /*/
		void											Remove();

		/** \brief Remove the attribute whose name and value matches with the passed ones.
		@param[in] name_attribute the name of the attribute to remove
		@param[in] value_attribute the new value of the attribute to remove
		* /*/
		void											Remove_Attribute(const std::string& name_attribute, const std::string& value_attribute);

		/** \brief Remove all the attributes with this tag.
		@param[in] name_attribute the name of the attributes to remove
		* /*/
		void											Remove_Attribute(const std::string& name_attribute);

		/** \brief Add an attribute with the passed name and value
		@param[in] name_attribute the name of the attribute to create
		@param[in] value_attribute the new value of the attribute to create
		* /*/
		void											Add_Attribute(const std::string& name_attribute, const std::string& contvalue_attributeent);

		/** \brief Add a nested tag with the passed name
		\details The tag created is empty, i.e. it has no attributes.
		@param[in] tag_name the name of the nested tag to create
		* /*/
		void											Add_Nested(const std::string& tag_name); 

		/** \brief Similar to Tag_readable::Add_Nested.
		\details Here the tag is created and returned in a Tag_readable.
		@param[in] tag_name the name of the nested tag to create
		* /*/
		Tag_readable							Add_Nested_and_return_created(const std::string& tag_name);

	private:
	// constructor
		Tag_readable(__Tag* enc) : encapsulated(enc) {};
		Tag_readable() : encapsulated(NULL) {};

		__Tag* encapsulated;
	};

	/** \brief Import the xml specified in the passed location.
	@param[in] source_file the location of the file to read, containing the xml structure to parse
	* /*/
	XML_reader(const std::string& source_file);

	/** \brief An empty structure is initialized using the default constructor.
	\details Only the root is created, i.e. a tag with a name 'Root' with no attributes. 
	You can use this  constructor to create an xml file from a process. In such a case, you have to use this
	constructor to initialize an empty structure. Then, you can get the root and add attributes ann nested tags using:
	Tag_readable::Add_field, Tag_readable::Add_Nested , Tag_readable::Add_Nested_and_return_created .
	* /*/
	XML_reader() { this->Tree_content = new __Tag("Root"); };

	~XML_reader() { delete this->Tree_content; };

	/** \brief Get the root of the structure.
	* /*/
	Tag_readable Get_root();

	/** \brief Print the actual structure in a textual file.
	@param[in] file_name the location of the file were to print the structure. It can be: a simle name, an absolute path or a relative path
	* /*/
	void		 Reprint(const std::string& file_name);

	/** \brief Similar to XML_reader::Reprint(const std::string& file_name)
	\details , but considering a stream to a file already opened: the structure is appended to the stream.
	@param[in] stream_to_use the stream to use
	* /*/
	void		 Reprint(std::ostream& stream_to_use);

	static void splitta_riga(std::string& riga, std::list<std::string>* slices);
private:
// data
	__Tag* Tree_content;	//only the root is allocated
};


#endif