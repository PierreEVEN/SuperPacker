#pragma once
#include <filesystem>
#include <optional>

namespace SuperPacker
{
	std::optional<std::filesystem::path> pick_file(const std::string& destination, std::vector<char> formats);
	std::optional<std::filesystem::path> save_file(std::vector<char> formats);
}
