#include "IniLoader.h"

#include <filesystem>
#include <iostream>
#include <fstream>

#include "Logger.h"

bool is_starting_with(const std::string& test, const std::string& start)
{
	for (int i = 0; i < start.size(); ++i) if (i >= test.size() || test[i] != start[i]) return false;
	return true;
}

bool is_ending_with(const std::string& test, const std::string& end)
{
	for (int64_t i = test.size() - 1; i > static_cast<int64_t>(test.size()) - static_cast<int64_t>(end.size()); ++i) if (i < 0 || test[i] != end[i]) return false;
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
			if (is_left) left += chr;
			else right += chr;
		}
	}
	else
	{
		is_left = false;
		is_right = true;
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
	return from_start ? (!is_left && is_right) : (!is_right && is_left);
}

IniLoader::IniLoader(const std::string file_path)
	: source_file(file_path) {
	link_or_create();
}

IniLoader::~IniLoader() {
	save();
	for (const auto& cat : ini_categories)
	{
		delete cat;
	}
	ini_categories.clear();
}

const std::string IniLoader::get_property_as_string(const std::string& categoryName, const std::string& propertyName, const std::string& defaultValue) {
	std::string props = get_property(categoryName, propertyName);
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

const int IniLoader::get_property_as_int(const std::string& categoryName, const std::string& propertyName, const int& defaultValue)
{
	std::string props = get_property(categoryName, propertyName);
	if (props != "")
	{
		return std::atoi(props.data());
	}
	return defaultValue;
}

const double IniLoader::get_property_as_double(const std::string& categoryName, const std::string& propertyName, const double& defaultValue)
{
	std::string props = get_property(categoryName, propertyName);
	if (props != "")
	{
		return std::atof(props.data());
	}
	return defaultValue;
}

const bool IniLoader::get_property_as_double(const std::string& categoryName, const std::string& propertyName, const bool& defaultValue)
{
	std::string props = get_property(categoryName, propertyName);
	if (props == "true") return true;
	else if (props == "false") return false;
	return defaultValue;
}

void IniLoader::save()
{
	std::ofstream of(source_file.data());

	for (const auto& cat : ini_categories)
	{
		std::string catString = cat->write_categories();
		of.write(catString.data(), catString.size());
	}
	of.close();
}

const std::string IniLoader::get_property(const std::string& categoryName, const std::string& propertyName) const
{
	for (const auto& cat : ini_categories)
	{
		if (cat->category_name == categoryName)
		{
			for (const auto& prop : cat->properties)
			{
				if (prop.property_name == propertyName)
				{
					return prop.value;
				}
			}
		}
	}
	return "";
}

void IniLoader::set_property(const std::string& categoryName, const std::string& propertyName, const std::string& propertyValue)
{
	if (propertyValue == "")
	{
		clear_property(categoryName, propertyName);
	}
	else
	{
		for (auto& category : ini_categories)
		{
			if (category->category_name == categoryName)
			{
				for (auto& prop : category->properties)
				{
					if (prop.property_name == propertyName)
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
		ini_categories.push_back(newCat);
		return;
	}
}

void IniLoader::clear_property(const std::string& categoryName, const std::string& propertyName)
{
	for (int i = (int)ini_categories.size() - 1; i >= 0; --i)
	{
		if (ini_categories[i]->category_name == categoryName)
		{
			for (int j = (int)ini_categories[i]->properties.size() - 1; j >= 0; --j)
			{
				if (ini_categories[i]->properties[j].property_name == propertyName)
				{
					ini_categories[i]->properties.erase(ini_categories[i]->properties.begin() + j);
				}
			}
			if (ini_categories[i]->properties.size() == 0)
			{
				delete ini_categories[i];
				ini_categories.erase(ini_categories.begin() + i);
			}
			return;
		}
	}
}

bool IniLoader::does_category_exists(const std::string& propertyName) const
{
	for (const auto& cat : ini_categories)
	{
		if (cat->category_name == propertyName) return true;
	}
	return false;
}

void IniLoader::link_or_create()
{
	if (!std::filesystem::exists(source_file.data()))
	{
		std::string left, right;
		split_string(source_file, { '/', '\'' }, left, right, false);
		std::filesystem::create_directories(left.data());
	}

	std::ifstream fs(source_file.data());
	char* line = new char[1000];
	IniCategory* currentCategory = nullptr;

	while (fs.getline(line, 1000, '\n'))
	{
		std::string resultLine, right;
		if (!split_string(line, { ';', '#' }, resultLine, right))
		{
			resultLine = line;
		}

		if (IniCategory::is_a_category_line(resultLine))
		{
			std::string catName = IniCategory::get_category_name_from_string(resultLine);

			if (!does_category_exists(catName))
			{
				currentCategory = new IniCategory(catName);
				ini_categories.push_back(currentCategory);
			}
			else
			{
				for (const auto& cat : ini_categories)
				{
					if (cat->category_name == catName) currentCategory = cat;
				}
			}
		}
		else
		{
			if (currentCategory && IniProperty::is_a_property_line(resultLine))
			{
				std::string name, value;
				IniProperty::get_property_name_and_value_from_string(line, name, value);
				if (!currentCategory->does_property_exists(name))
				{
					currentCategory->add_property(name, value);
				}
			}
		}
	}
	delete[] line;
	fs.close();
}

IniLoader::IniProperty::IniProperty(const std::string& inPropertyName, const std::string& inPropertyValue) {
	property_name = inPropertyName;
	value = inPropertyValue;
}

std::string IniLoader::IniProperty::write_line() const {
	return property_name+ '=' + value + '\n';
}

void IniLoader::IniProperty::get_property_name_and_value_from_string(const std::string& line, std::string& name, std::string& value)
{
	split_string(line, { '=' }, name, value);
	name = trim(name);
	value = trim(value);
}

bool IniLoader::IniProperty::is_a_property_line(const std::string& line)
{
	std::string left, right;
	return (split_string(line, { '=' }, left, right, true) && right != "");
}

IniLoader::IniCategory::IniCategory(const std::string& in_category_name)
{
	category_name = in_category_name;
}

void IniLoader::IniCategory::add_property(const std::string& propertyName, const std::string& propertyValue)
{
	properties.push_back(IniProperty(propertyName, propertyValue));
}

std::string IniLoader::IniCategory::write_categories() const
{
	std::string outString = '[' + category_name + "]\n";
	for (const auto& prop : properties)
	{
		outString += prop.write_line();
	}
	return outString + "\n";
}

bool IniLoader::IniCategory::does_property_exists(const std::string& propertyName) const
{
	for (const auto& prop : properties)
	{
		if (prop.property_name == propertyName) return true;
	}
	return false;
}

const std::string IniLoader::IniCategory::get_category_name_from_string(const std::string& line)
{
	std::string left, center, right, categoryName;
	split_string(line, { '[' }, left, center);
	split_string(center, { ']' }, categoryName, right);
	
	return categoryName;
}

bool IniLoader::IniCategory::is_a_category_line(const std::string& line)
{
	return is_starting_with(line, "[") && is_ending_with(line, "]");
}
