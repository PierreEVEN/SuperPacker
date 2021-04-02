#include "IniLoader.h"
#if _WIN32
	#include <filesystem>
#endif
#include <iostream>
#include <fstream>

#include "logger.h"

bool is_starting_with(const std::string& test, const std::string& start)
{
	for (int i = 0; i < start.size(); ++i) if (i >= test.size() || test[i] != start[i]) return false;
	return true;
}

std::string trim(const std::string& source)
{
	std::string target = source;
	target.erase(target.begin(), std::ranges::find_if(target.begin(), target.end(), [](const unsigned char ch) {
		return !std::isspace(ch);
		}));
	target.erase(std::find_if(target.rbegin(), target.rend(), [](const unsigned char ch) {
		return !std::isspace(ch);
		}).base(), target.end());
	return target;
}

bool split_string(const std::string& test, const std::vector<char>& separators, std::string& left, std::string& right, bool from_start = true)
{
	left = "";
	bool is_left = true;
	bool is_right = false;
	right = "";

	if (from_start) {
		for (const auto& chr : test)
		{
			if (std::ranges::find(separators.begin(), separators.end(), chr) != separators.end())
			{
				if (is_left) {
					is_left = false;
					is_right = true;
					continue;
				}
			}
			if (is_left) left = chr + left;
			else right = chr + right;
		}
	}
	else
	{
		bool is_left = false;
		bool is_right = true;
		for (int64_t i = test.size() - 1; i >= 0; --i)
		{
			if (std::ranges::find(separators.begin(), separators.end(), test[i]) != separators.end())
			{
				if (is_right) {
					is_left = true;
					is_right = false;
					continue;
				}
			}
			if (is_left) left = test[i] + left;
			else right = test[i] + right;
		}
	}
	return !is_left && is_right;
}

IniLoader::IniLoader(const std::string& filePath)
	: sourceFile(filePath) {
	LinkOrCreate();
}

IniLoader::~IniLoader() {
	Save();
	for (const auto& cat : iniCategories)
	{
		delete cat;
	}
	iniCategories.clear();
}

const std::string IniLoader::GetPropertyAsString(const char* categoryName, const char* propertyName, const char* defaultValue) {
	std::string props = GetProperty(categoryName, propertyName);
	if (props != "")
	{
		std::string left, center, right, finalS;
		if (split_string(props, { '"' }, left, center, true))
		{
			if (split_string(center, { '"' }, finalS, right, false))
			{
				return finalS.data();
			}
		}
	}
	return defaultValue;
}

const int IniLoader::GetPropertyAsInt(const char* categoryName, const char* propertyName, const int& defaultValue)
{
	std::string props = GetProperty(categoryName, propertyName);
	if (props != "")
	{
		return std::atoi(props.data());
	}
	return defaultValue;
}

const double IniLoader::GetPropertyAsDouble(const char* categoryName, const char* propertyName, const double& defaultValue)
{
	std::string props = GetProperty(categoryName, propertyName);
	if (props != "")
	{
		return std::atof(props.data());
	}
	return defaultValue;
}

const bool IniLoader::GetPropertyAsBool(const char* categoryName, const char* propertyName, const bool& defaultValue)
{
	std::string props = GetProperty(categoryName, propertyName);
	if (props == "true") return true;
	else if (props == "false") return false;
	return defaultValue;
}

void IniLoader::Save()
{
	std::ofstream of(sourceFile.data());

	for (const auto& cat : iniCategories)
	{
		std::string catString = cat->WriteCategories();
		of.write(catString.data(), catString.size());
	}
	of.close();
}

const std::string IniLoader::GetProperty(const std::string& categoryName, const std::string& propertyName) const
{
	for (const auto& cat : iniCategories)
	{
		if (cat->categoryName == categoryName)
		{
			for (const auto& prop : cat->properties)
			{
				if (prop.propertyName == propertyName)
				{
					return prop.value;
				}
			}
		}
	}
	return "";
}

void IniLoader::SetProperty(const std::string& categoryName, const std::string& propertyName, const std::string& propertyValue)
{
	if (propertyValue == "")
	{
		ClearProperty(categoryName, propertyName);
	}
	else
	{
		for (auto& category : iniCategories)
		{
			if (category->categoryName == categoryName)
			{
				for (auto& prop : category->properties)
				{
					if (prop.propertyName == propertyName)
					{
						prop.value = propertyValue;
						return;
					}
				}
				category->properties.push_back(IniProperty(propertyName, propertyValue));
				return;
			}
		}
		IniCategory* newCat = new IniCategory(categoryName);
		newCat->properties.push_back(IniProperty(propertyName, propertyValue));
		iniCategories.push_back(newCat);
		return;
	}
}

void IniLoader::ClearProperty(const std::string& categoryName, const std::string& propertyName)
{
	for (int i = (int)iniCategories.size() - 1; i >= 0; --i)
	{
		if (iniCategories[i]->categoryName == categoryName)
		{
			for (int j = (int)iniCategories[i]->properties.size() - 1; j >= 0; --j)
			{
				if (iniCategories[i]->properties[j].propertyName == propertyName)
				{
					iniCategories[i]->properties.erase(iniCategories[i]->properties.begin() + j);
				}
			}
			if (iniCategories[i]->properties.size() == 0)
			{
				delete iniCategories[i];
				iniCategories.erase(iniCategories.begin() + i);
			}
			return;
		}
	}
}

bool IniLoader::DoesCategoryExist(const std::string& propertyName) const
{
	for (const auto& cat : iniCategories)
	{
		if (cat->categoryName == propertyName) return true;
	}
	return false;
}

void IniLoader::LinkOrCreate()
{
	if (!std::filesystem::exists(sourceFile.data()))
	{
		std::string left, right;
		split_string(sourceFile, { '/', '\'' }, left, right, false);
		std::filesystem::create_directories(left.data());
	}

	std::ifstream fs(sourceFile.data());
	char* line = new char[1000];
	IniCategory* currentCategory = nullptr;
	while (fs.getline(line, 1000, '\n'))
	{
		std::string resultLine, right;
		if (!split_string(line, { ';', '#' }, resultLine, right))
		{
			resultLine = line;
		}


		if (IniCategory::IsCategoryLine(resultLine))
		{
			std::string catName = IniCategory::GetCategoryNameFromString(resultLine);
			if (!DoesCategoryExist(catName))
			{
				currentCategory = new IniCategory(catName);
				iniCategories.push_back(currentCategory);
			}
			else
			{
				for (const auto& cat : iniCategories)
				{
					if (cat->categoryName == catName) currentCategory = cat;
				}
			}
		}
		else
		{
			if (currentCategory && IniProperty::IsPropertyLine(resultLine))
			{
				std::string name, value;
				IniProperty::GetPropertyNameAndValueFromString(line, name, value);
				if (!currentCategory->DoesPropertyExist(name))
				{
					currentCategory->AddProperty(name, value);
				}
			}
		}
	}
	delete line;
	fs.close();
}

IniLoader::IniProperty::IniProperty(const std::string& inPropertyName, const std::string& inPropertyValue) {
	propertyName = inPropertyName;
	value = inPropertyValue;
}

std::string IniLoader::IniProperty::WriteLine() const {
	return propertyName+ '=' + value + '\n';
}

void IniLoader::IniProperty::GetPropertyNameAndValueFromString(const std::string& line, std::string& name, std::string& value)
{
	split_string(line, { '=' }, name, value);
	name = trim(name);
	value = trim(value);
}

bool IniLoader::IniProperty::IsPropertyLine(const std::string& line)
{
	std::string left, right;
	return (split_string(line, { '=' }, left, right, true) && right != "");
}

IniLoader::IniCategory::IniCategory(const std::string& inCategoryName)
{
	categoryName = inCategoryName;
}

void IniLoader::IniCategory::AddProperty(const std::string& propertyName, const std::string& propertyValue)
{
	properties.push_back(IniProperty(propertyName, propertyValue));
}

std::string IniLoader::IniCategory::WriteCategories() const
{
	std::string outString = '[' + categoryName + "]\n";
	for (const auto& prop : properties)
	{
		outString += prop.WriteLine();
	}
	return outString + "\n";
}

bool IniLoader::IniCategory::DoesPropertyExist(const std::string& propertyName) const
{
	for (const auto& prop : properties)
	{
		if (prop.propertyName == propertyName) return true;
	}
	return false;
}

const std::string IniLoader::IniCategory::GetCategoryNameFromString(const std::string& line)
{
	std::string left, center, right, categoryName;
	split_string(line, { '[' }, left, center);
	split_string(center, { ']' }, categoryName, right);
	return categoryName;
}

bool IniLoader::IniCategory::IsCategoryLine(const std::string& line)
{
	return is_starting_with(line, "[") && is_starting_with(line, "]");
}
