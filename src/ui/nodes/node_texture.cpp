#include "node_texture.h"

#include <gl/gl3w.h>

#include "gfx.h"
#include "texture.h"

void NodeTexture::display()
{
}

void NodeTexture::register_uniform(CodeContext& ctx)
{
	Node::register_uniform(ctx);
	shader_uniform = ctx.add_uniform(EType::Sampler2D);
	shader_uniform->on_update_value.add_lambda([&](int location)
	{
		if (texture)
		{
			glUniform1i(location, location);
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D, texture->get_id());
			GL_CHECK_ERROR();
		}
	});
}

void NodeTexture::load_or_reload()
{
	if (!path->target() || path->target()->on_get_type.execute() != EType::String)
	{
		texture = nullptr;
		return;
	}

	const std::filesystem::path file = path->target()->on_get_code.execute(get_graph().code_ctx());

	if (!exists(file))
	{
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
		return std::format("{} = texture2D({}, text_coords); {}.a += 0.2;", context.glsl_output_var(EType::Float4),
		                   shader_uniform->get_name(), context.glsl_output_var(EType::Float4));
	});

	auto r = add_output("R");
	r->on_get_type.add_lambda([]() { return EType::Float; });
	r->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		load_or_reload();
		return std::format("{} = texture2D({}, text_coords).r;", context.glsl_output_var(EType::Float),
		                   shader_uniform->get_name());
	});

	auto g = add_output("G");
	g->on_get_type.add_lambda([]() { return EType::Float; });
	g->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		load_or_reload();
		return std::format("{} = texture2D({}, text_coords).g;", context.glsl_output_var(EType::Float),
		                   shader_uniform->get_name());
	});

	auto b = add_output("B");
	b->on_get_type.add_lambda([]() { return EType::Float; });
	b->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		load_or_reload();
		return std::format("{} = texture2D({}, text_coords).b;", context.glsl_output_var(EType::Float),
		                   shader_uniform->get_name());
	});

	auto a = add_output("A");
	a->on_get_type.add_lambda([]() { return EType::Float; });
	a->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		load_or_reload();
		return std::format("{} = texture2D({}, text_coords).a;", context.glsl_output_var(EType::Float),
		                   shader_uniform->get_name());
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
