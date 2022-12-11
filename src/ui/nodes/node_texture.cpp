#include "node_texture.h"

#include <gl/gl3w.h>

#include "gfx.h"
#include "texture.h"

/*
 * TEXTURE INPUT
 */

NodeTexture::NodeTexture()
{
	const auto col = add_output("RGBA");
	col->on_get_type.add_lambda([]() { return EType::Float4; });
	col->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		return std::format("{} = vec4(texture2D({}, vec2(text_coords.x, 1 - text_coords.y)) * {});",
		                   context.glsl_output_var(EType::Float4),
		                   texture_uniform->get_name(), enabled_uniform->get_name());
	});

	const auto r = add_output("R");
	r->on_get_type.add_lambda([]() { return EType::Float; });
	r->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		return std::format("{} = texture2D({}, vec2(text_coords.x, 1 - text_coords.y)).r * {};",
		                   context.glsl_output_var(EType::Float),
		                   texture_uniform->get_name(), enabled_uniform->get_name());
	});

	const auto g = add_output("G");
	g->on_get_type.add_lambda([]() { return EType::Float; });
	g->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		return std::format("{} = texture2D({}, vec2(text_coords.x, 1 - text_coords.y)).g * {};",
		                   context.glsl_output_var(EType::Float),
		                   texture_uniform->get_name(), enabled_uniform->get_name());
	});

	const auto b = add_output("B");
	b->on_get_type.add_lambda([]() { return EType::Float; });
	b->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		return std::format("{} = texture2D({}, vec2(text_coords.x, 1 - text_coords.y)).b * {};",
		                   context.glsl_output_var(EType::Float),
		                   texture_uniform->get_name(), enabled_uniform->get_name());
	});

	const auto a = add_output("A");
	a->on_get_type.add_lambda([]() { return EType::Float; });
	a->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		return std::format("{} = texture2D({}, vec2(text_coords.x, 1 - text_coords.y)).a * {};",
		                   context.glsl_output_var(EType::Float),
		                   texture_uniform->get_name(), enabled_uniform->get_name());
	});

	path = add_input("path");

	texture_data.on_data_changed.add_lambda([&]
	{
		if (texture_data.is_valid())
		{
			texture = Texture::create();
			texture->load_from_disc(texture_data.get_path());
		}
		else
		{
			texture = nullptr;
		}
	});

	on_update.add_lambda([&]
	{
		if (*path)
			texture_data.set_path(path->target()->get_code(get_graph().code_ctx()));
		else
			texture_data.set_path("");
	});
}

NodeTexture::~NodeTexture()
{
	texture_uniform->unregister();
	enabled_uniform->unregister();
}

void NodeTexture::register_uniform(CodeContext& ctx)
{
	Node::register_uniform(ctx);
	texture_uniform = ctx.add_uniform(EType::Sampler2D);
	texture_uniform->on_update_value.add_lambda([&](int location)
	{
		if (texture && texture->ready())
		{
			glUniform1i(location, location);
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D, texture->get_id());
			glGetError();
			GL_CHECK_ERROR();
		}
	});

	enabled_uniform = ctx.add_uniform(EType::Int);
	enabled_uniform->on_update_value.add_lambda([&](int location)
	{
		glUniform1i(location, texture && texture->ready() ? 1 : 0);
		glGetError();
		GL_CHECK_ERROR();
	});
}

void NodeTexture::display()
{
}

/*
 * TEXTURE OUTPUT
 */

ImageWriteNode::ImageWriteNode()
{
	rgba_input = add_input("RGBA");
	path_input = add_input("Path");

	const auto output = add_output("Color");
	output->on_get_type.add_lambda([]() { return EType::Float4; });
	output->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (rgba_input->target())
		{
			auto rgba_code = rgba_input->target()->on_get_code.execute(context);
			return std::format("{}\n", rgba_code);
		}
		return "";
	});
}

void ImageWriteNode::display()
{
	if (!path_input->target() || path_input->target()->get_type() != EType::String)
		return;

	if (ImGui::Button("Export", ImGui::GetContentRegionAvail()))
	{
		std::cout << "output to " << path_input->target()->get_code(get_graph().code_ctx()) << std::endl;
	}
}

void ImageWriteNode::display_summary()
{	
	if (ImGui::Button("Export", {ImGui::GetContentRegionAvail().x, 100}))
	{
		std::cout << "output to " << path_input->target()->get_code(get_graph().code_ctx()) << std::endl;
	}
}

REGISTER_NODE(ImageWriteNode, NodeInfo("", {"Image Write"}));
REGISTER_NODE(NodeTexture, NodeInfo("", {"Texture"}));