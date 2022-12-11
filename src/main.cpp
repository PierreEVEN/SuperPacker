#include "gfx.h"

#include "file_resource.h"
#include "nfd.hpp"
#include "window_interface.h"
#include "ui/graph_browser.h"


int main(int argc, char** argv)
{
	// Find user save dir
	auto roaming_dir = windows::get_user_metadata_dir();
	if (!exists(roaming_dir))
		roaming_dir = std::filesystem::path(argv[0]).parent_path();
	roaming_dir = roaming_dir / "SuperPacker" / "Saved";

	NFD::Init();

	// Load saved config
	GraphManager browser(roaming_dir);

	// Init window
	int pos_x, pos_y;
	const bool valid_pos = browser.get_saved_window_pos(pos_x, pos_y);
	Gfx gfx(
		"Super Packer - OpenGL - v3.0",
		browser.get_saved_window_width(),
		browser.get_saved_window_height(),
		valid_pos ? &pos_x : nullptr,
		valid_pos ? &pos_y : nullptr
	);

	// Load layout
	browser.load_layout();

	// Draw UI
	gfx.on_draw.add_lambda([&]
	{
		FileResource::refresh_file_resources();
		browser.display();
	});

	// Render loop
	while (gfx.draw());

	// Save state before quitting
	browser.save_layout();

	NFD::Quit();
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return main(0, nullptr);
}
