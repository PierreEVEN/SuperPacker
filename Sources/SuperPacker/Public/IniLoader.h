#pragma once

#include <string>
#include <vector>

/*
 * @IniLoader - Read and write ini files
 *
 * 1) Create a new IniLoader with desired file path
 *			IniLoader my_loader("path/config.ini");
 *
 * 2) Read and write property from your loader
 *			std::string value = myLoader.get_property_as_string("category_name", "property_name", "defaultValue");
 *			myLoader.set_property_as_string("category_name", "property_name", "value");
 *
 * 3) Clear a property value by writing an empty string
 *			myLoader.set_property_as_string("category_name", "property_name", "");
 *
 * 3) Don't forget to save modifications
 *			myLoader.save();
 *
 */


class IniLoader final
{
public:

	/** Create a new ini loader - automatically load file at designed path or create path if file doesn't exist. Ini file must have '.ini extension' */
	IniLoader(const std::string file_path);

	/** save ini modifications, then delete resources */
	~IniLoader();

	/** Get ini property from category and property name. Default value is returned if we can't find any occurrence */
	[[nodiscard]] const std::string get_property_as_string(const std::string& categoryName, const std::string& propertyName, const std::string& defaultValue = "");
	[[nodiscard]] const double get_property_as_double(const std::string& categoryName, const std::string& propertyName, const double& defaultValue = 0.f);
	[[nodiscard]] const int get_property_as_int(const std::string& categoryName, const std::string& propertyName, const int& defaultValue = 0);
	[[nodiscard]] const bool get_property_as_double(const std::string& categoryName, const std::string& propertyName, const bool& defaultValue = false);

	/** Set ini property (Save() must be called to save modifications on disk) */
	void set_property_as_string(const std::string& categoryName, const std::string& propertyName, const std::string& propertyValue) { set_property(categoryName, propertyName, std::string('"' + std::string(propertyValue) + '"')); }
	void set_property_as_double(const std::string& categoryName, const std::string& propertyName, const double& propertyValue) { set_property(categoryName, propertyName, std::to_string(propertyValue)); }
	void set_property_as_int(const std::string& categoryName, const std::string& propertyName, const int& propertyValue) { set_property(categoryName, propertyName, std::to_string(propertyValue)); }
	void set_property_as_bool(const std::string& categoryName, const std::string& propertyName, const bool& propertyValue) { set_property(categoryName, propertyName, propertyValue ? "true" : "false"); }

	/** Create or update ini file on disk */
	void save();

private:

	/** Ini structures */
	struct IniProperty
	{
		IniProperty(const std::string& propertyName, const std::string& propertyValue);

		std::string property_name;
		std::string value;

		[[nodiscard]] std::string write_line() const;
		static void get_property_name_and_value_from_string(const std::string& line, std::string& name, std::string& value);
		static bool is_a_property_line(const std::string& line);
	};

	struct IniCategory
	{
		IniCategory(const std::string& in_category_name);

		std::string category_name;
		std::vector<IniProperty> properties;

		void add_property(const std::string& propertyName, const std::string& propertyValue);
		[[nodiscard]] std::string write_categories() const;

		bool does_property_exists(const std::string& propertyName) const;
		static const std::string get_category_name_from_string(const std::string& line);
		static bool is_a_category_line(const std::string& line);
	};

	/** Internal methods */
	[[nodiscard]] const std::string get_property(const std::string& categoryName, const std::string& propertyName) const;
	void set_property(const std::string& categoryName, const std::string& propertyName, const std::string& propertyValue);
	void clear_property(const std::string& categoryName, const std::string& propertyName);
	[[nodiscard]] bool does_category_exists(const std::string& propertyName) const;

	void link_or_create();

	/** ini file path */
	std::string source_file;

	/** Ini categories (each category contains a property vector) */
	std::vector<IniCategory*> ini_categories;
};
