#include "file_resource.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static std::vector<FileResource*> file_resources;

FileResource::FileResource()
{
	on_data_changed.add_lambda([&]
		{
			file_data.clear();
		});
	file_resources.emplace_back(this);
}

FileResource::~FileResource()
{
	file_resources.erase(std::find(file_resources.begin(), file_resources.end(), this));
}

void FileResource::set_path(const std::string& path)
{
	if (file_path != path)
		file_path = path;
	if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path))
	{
		if (!is_valid_path)
		{
			last_write_time = std::filesystem::last_write_time(path);
			is_valid_path = true;
			on_data_changed();
		}
	}
	else
	{
		if (is_valid_path)
		{
			is_valid_path = false;
			last_write_time = {};
			on_data_changed();
		}
	}
}

void FileResource::try_refresh()
{
	set_path(file_path);

	if (is_valid() && last_write_time != std::filesystem::last_write_time(file_path))
	{
		last_write_time = std::filesystem::last_write_time(file_path);
		on_data_changed();
	}
}

const std::vector<char>& FileResource::get_raw_data()
{
	if (is_valid())
	{
		if (file_data.empty())
		{
			std::ifstream file_input(file_path);
			if (file_input.is_open())
			{
				file_data.resize(file_input.tellg());
				if (file_input.read(file_data.data(), file_input.tellg()))
				{
					return file_data;
				}
			}
			std::cerr << "failed to read file content" << std::endl;
		}
	}
	return file_data;
}

void FileResource::refresh_file_resources()
{
	for (const auto& resource : file_resources)
		resource->try_refresh();
}
