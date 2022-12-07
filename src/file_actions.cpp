#include "file_actions.h"

#include <iostream>
#include <Windows.h>

std::optional<std::filesystem::path> pick_file(const std::string& search_dir, std::vector<char> formats)
{
	const std::string title = ("Select file for channel " + search_dir).c_str();

	const std::string any_file_text = "any file";
	const std::string any_file_format = "*.*";

	for (const auto& chr : any_file_text) formats.push_back(chr);
	formats.push_back('\0');
	for (const auto& chr : any_file_format) formats.push_back(chr);
	formats.push_back('\0');
	formats.push_back('\0');

	OPENFILENAME ofn_infos;
	char filename[MAX_PATH];
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn_infos, sizeof(ofn_infos));
	ofn_infos.lStructSize = sizeof(ofn_infos);
	ofn_infos.hwndOwner = NULL;
	ofn_infos.lpstrFilter = formats.data();
	ofn_infos.lpstrFile = filename;
	ofn_infos.nMaxFile = MAX_PATH;
	ofn_infos.lpstrTitle = title.c_str();
	ofn_infos.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn_infos))
	{
		return std::filesystem::path(filename);
	}
	switch (CommDlgExtendedError())
	{
	case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";
		break;
	case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";
		break;
	case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";
		break;
	case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";
		break;
	case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";
		break;
	case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";
		break;
	case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n";
		break;
	case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";
		break;
	case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";
		break;
	case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";
		break;
	case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";
		break;
	case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";
		break;
	case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";
		break;
	case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n";
		break;
	case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n";
		break;
	default: break;
	}
	return {};
}

#include <ShlObj.h>

using namespace std;
void test()
{
	char selectedPath[MAX_PATH] = { 0 };
	bool result = false;

	// Initialize the OPENFILENAME structure
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = selectedPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_NONETWORKBUTTON | OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = "Select a folder";

	// Show the "Select Folder" dialog
	if (GetOpenFileName(&ofn))
	{
		result = true;
	}

}
