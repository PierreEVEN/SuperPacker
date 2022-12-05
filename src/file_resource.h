#pragma once

#include <event_manager.h>
#include <filesystem>
#include <string>


DECLARE_DELEGATE_MULTICAST(EventFileChanged);

class FileResource
{
public:
	FileResource();
	virtual ~FileResource();

	EventFileChanged on_data_changed;

	static void refresh_file_resources();

	void set_path(const std::string& path);
	[[nodiscard]] const std::vector<char>& get_raw_data();
	[[nodiscard]] std::string get_path() { return is_valid_path ? file_path : ""; }
	[[nodiscard]] bool is_valid() const { return is_valid_path; }

private:
	void try_refresh();

	bool is_valid_path = false;
	std::string file_path;
	std::vector<char> file_data;
	std::filesystem::file_time_type last_write_time;
};
