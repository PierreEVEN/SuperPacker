#pragma once

#include <string>
#include <vector>

/*
 * @IniLoader - Read and write ini files
 *
 * 1) Create a new IniLoader with desired file path
 *			IniLoader myLoader("path/config.ini");
 *
 * 2) Read and write property from your loader
 *			String value = myLoader.GetPropertyAsString("categoryName", "propertyName", "defaultValue");
 *			myLoader.SetPropertyAsString("categoryName", "propertyName", "value");
 *
 * 3) Clear a property value by writing an empty string
 *			myLoader.SetPropertyAsString("categoryName", "propertyName", "");
 *
 * 3) Don't forget to save modifications
 *			myLoader.Save();
 *
 */


class IniLoader
{
public:

	/** Create a new ini loader - automatically load file at designed path or create path if file doesn't exist. Ini file must have '.ini extension' */
	IniLoader(const std::string filePath);

	/** save ini modifications, then delete resources */
	~IniLoader();

	/** Get ini property from category and property name. Default value is returned if we can't find any occurrence */
	const std::string GetPropertyAsString(const char* categoryName, const char* propertyName, const char* defaultValue = "");
	const double GetPropertyAsDouble(const char* categoryName, const char* propertyName, const double& defaultValue = 0.f);
	const int GetPropertyAsInt(const char* categoryName, const char* propertyName, const int& defaultValue = 0);
	const bool GetPropertyAsBool(const char* categoryName, const char* propertyName, const bool& defaultValue = false);

	/** Set ini property (Save() must be called to save modifications on disk) */
	void SetPropertyAsString(const char* categoryName, const char* propertyName, const char* propertyValue) { SetProperty(categoryName, propertyName, std::string('"' + std::string(propertyValue) + '"'))	; }
	void SetPropertyAsDouble(const char* categoryName, const char* propertyName, const double& propertyValue) { SetProperty(categoryName, propertyName, std::to_string(propertyValue)); }
	void SetPropertyAsInt(const char* categoryName, const char* propertyName, const int& propertyValue) { SetProperty(categoryName, propertyName, std::to_string(propertyValue)); }
	void SetPropertyAsBool(const char* categoryName, const char* propertyName, const bool& propertyValue) { SetProperty(categoryName, propertyName, propertyValue ? "true" : "false"); }

	/** Create or update ini file on disk */
	void Save();

private:

	/** Ini structures */
	struct IniProperty
	{
		IniProperty(const std::string& propertyName, const std::string& propertyValue);

		std::string propertyName;
		std::string value;

		std::string WriteLine() const;
		static void GetPropertyNameAndValueFromString(const std::string& line, std::string& name, std::string& value);
		static bool IsPropertyLine(const std::string& line);
	};

	struct IniCategory
	{
		IniCategory(const std::string& categoryname);

		std::string categoryName;
		std::vector<IniProperty> properties;

		void AddProperty(const std::string& propertyName, const std::string& propertyValue);
		std::string WriteCategories() const;

		bool DoesPropertyExist(const std::string& propertyName) const;
		static const std::string GetCategoryNameFromString(const std::string& line);
		static bool IsCategoryLine(const std::string& line);
	};

	/** Internal methods */
	const std::string GetProperty(const std::string& categoryName, const std::string& propertyName) const;
	void SetProperty(const std::string& categoryName, const std::string& propertyName, const std::string& propertyValue);
	void ClearProperty(const std::string& categoryName, const std::string& propertyName);
	bool DoesCategoryExist(const std::string& propertyName) const;

	void LinkOrCreate();

	/** ini file path */
	std::string sourceFile;

	/** Ini categories (each category contains a property vector) */
	std::vector<IniCategory*> iniCategories;
};
