#include "gfx.h"

#include "file_resource.h"
#include "nfd.hpp"
#include "ui/graph_browser.h"


int main(int argc, char** argv)
{
	NFD::Init();
	GraphBrowser browser;
	browser.load_defaults("default");

	int pos_x, pos_y;
	const bool valid_pos = browser.get_saved_window_pos(pos_x, pos_y);
	Gfx gfx("Super Packer v3.0", browser.get_saved_window_width(), browser.get_saved_window_height(),
	        valid_pos ? &pos_x : nullptr, valid_pos ? &pos_y : nullptr);
	browser.load_layout();
	browser.new_graph("test");
	browser.new_graph("Demo2");

	gfx.on_draw.add_lambda([&]
	{
		FileResource::refresh_file_resources();
		browser.display();
	});

	while (gfx.draw());
	browser.save_layout();
	NFD::Quit();
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return main(0, nullptr);
}
