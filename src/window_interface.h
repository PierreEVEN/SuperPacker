#pragma once
#include "ui/nodes/node_maths.h"

namespace windows
{

	std::filesystem::path get_user_metadata_dir();

	std::filesystem::path pick_graph_file();
	std::filesystem::path pick_image();

	std::filesystem::path create_graph_file(const std::filesystem::path& base_path);


}
