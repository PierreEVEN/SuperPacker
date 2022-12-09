#include "window_interface.h"

#include <Windows.h>
#include <Shlobj.h>

#include <nfd.hpp>

std::filesystem::path windows::get_user_metadata_dir()
{
	PWSTR path_tmp;
	if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path_tmp) != S_OK)
	{
		CoTaskMemFree(path_tmp);
		return {};
	}

	std::filesystem::path path = path_tmp;
	CoTaskMemFree(path_tmp);
	return path;
}

std::filesystem::path windows::pick_graph_file()
{
	nfdnchar_t* outPath;
	const std::vector<nfdnfilteritem_t> filters = {
		{
			.name = L"Super Packer Graph",
			.spec = L"spg"
		}
	};
	if (NFD::OpenDialog(outPath, filters.data(), static_cast<nfdfiltersize_t>(filters.size())) == NFD_OKAY)
	{
		std::filesystem::path result(outPath);
		NFD::FreePath(outPath);
		return result;
	}
	return {};
}