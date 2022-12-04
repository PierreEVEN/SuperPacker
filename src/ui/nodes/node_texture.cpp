
#include "node_texture.h"

#include "texture.h"

void NodeTexture::display()
{
	if (texture)
		ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(texture->get_id())),
			ImGui::GetContentRegionAvail());
}

void NodeTexture::load_or_reload()
{
	if (!path->input || path->input->on_get_type.execute() != EType::String) {
		texture = nullptr;
		return;
	}
	
	CodeContext ctx;
	const std::filesystem::path file = path->input->on_get_code.execute(ctx);

	if (!exists(file)) {
		texture = nullptr;
		return;
	}

	if (!texture || texture->get_path() != file)
	{
		texture = Texture::create(file);
	}
}

void ImageWriteNode::display()
{
}

NodeTexture::NodeTexture()
	: Node("Texture")
{
	auto col = add_output("RGBA");
	col->on_get_type.add_lambda([]() { return EType::Float4; });
	col->on_get_code.add_lambda([&](CodeContext& context)-> std::string
		{
			load_or_reload();
			const auto uname = context.generate_name();
			context.add_uniform(uname, EType::Sampler2D);
			return std::format("{} = texture2D({}, text_coords);", context.glsl_output_var(EType::Float4), uname);
		});

	auto r = add_output("R");
	r->on_get_type.add_lambda([]() { return EType::Float; });
	r->on_get_code.add_lambda([&](CodeContext& context)-> std::string
		{
			load_or_reload();
			const auto uname = context.generate_name();
			context.add_uniform(uname, EType::Sampler2D);
			return std::format("{} = texture2D({}, text_coords).r;", context.glsl_output_var(EType::Float), uname);
		});

	auto g = add_output("G");
	g->on_get_type.add_lambda([]() { return EType::Float; });
	g->on_get_code.add_lambda([&](CodeContext& context)-> std::string
		{
			load_or_reload();
			const auto uname = context.generate_name();
			context.add_uniform(uname, EType::Sampler2D);
			return std::format("{} = texture2D({}, text_coords).g;", context.glsl_output_var(EType::Float), uname);
		});

	auto b = add_output("B");
	b->on_get_type.add_lambda([]() { return EType::Float; });
	b->on_get_code.add_lambda([&](CodeContext& context)-> std::string
		{
			load_or_reload();
			const auto uname = context.generate_name();
			context.add_uniform(uname, EType::Sampler2D);
			return std::format("{} = texture2D({}, text_coords).b;", context.glsl_output_var(EType::Float), uname);
		});

	auto a = add_output("A");
	a->on_get_type.add_lambda([]() { return EType::Float; });
	a->on_get_code.add_lambda([&](CodeContext& context)-> std::string
		{
			load_or_reload();
			const auto uname = context.generate_name();
			context.add_uniform(uname, EType::Sampler2D);
			return std::format("{} = texture2D({}, text_coords).a;", context.glsl_output_var(EType::Float), uname);
		});

	path = add_input("path");
}

TextureResizeNode::TextureResizeNode() : Node("Resize")
{
	color = add_input("channel");
	in_x = add_input("size_x");
	in_y = add_input("size_y");
	auto out = add_output("result");
}
