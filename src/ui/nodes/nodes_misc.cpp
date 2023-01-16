#include "nodes_misc.h"

#include "ui/pin.h"

MakeFloat4::MakeFloat4()
{
	r = add_input("r");
	g = add_input("g");
	b = add_input("b");
	a = add_input("a");

	const auto out = add_output("result");


	out->on_get_type.add_lambda([] { return EType::Float4; });

	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!r->is_connected() || !g->is_connected() || !b->is_connected() || !a->is_connected() ||
			r->target()->get_type() != EType::Float ||
			g->target()->get_type() != EType::Float ||
			b->target()->get_type() != EType::Float ||
			a->target()->get_type() != EType::Float)
			return "";

		const auto r_code = r->target()->get_code(context);
		const auto g_code = g->target()->get_code(context);
		const auto b_code = b->target()->get_code(context);
		const auto a_code = a->target()->get_code(context);

		return std::format(
			"vec4 result = vec4(0,0,0,1);\n"
			"{{{}}}\n"
			"result.r = {};\n"
			"{{{}}}\n"
			"result.g = {};\n"
			"{{{}}}\n"
			"result.b = {};\n"
			"{{{}}}\n"
			"result.a = {};\n"
			"{} = result;",
			r_code,
			context.glsl_output_var(EType::Float),
			g_code,
			context.glsl_output_var(EType::Float),
			b_code,
			context.glsl_output_var(EType::Float),
			a_code,
			context.glsl_output_var(EType::Float),
			context.glsl_output_var(EType::Float4)
		);
	});
}

MakeFloat3::MakeFloat3()
{
	r = add_input("r");
	g = add_input("g");
	b = add_input("b");

	const auto out = add_output("result");


	out->on_get_type.add_lambda([] { return EType::Float3; });

	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!r->is_connected() || !g->is_connected() || !b->is_connected() ||
			r->target()->get_type() != EType::Float ||
			g->target()->get_type() != EType::Float ||
			b->target()->get_type() != EType::Float)
			return "";

		const auto r_code = r->target()->get_code(context);
		const auto g_code = g->target()->get_code(context);
		const auto b_code = b->target()->get_code(context);

		return std::format(
			"vec4 result = vec4(0,0,0,1);\n"
			"{{{}}}\n"
			"result.r = {};\n"
			"{{{}}}\n"
			"result.g = {};\n"
			"{{{}}}\n"
			"result.b = {};\n"
			"{} = result;",
			r_code,
			context.glsl_output_var(EType::Float),
			g_code,
			context.glsl_output_var(EType::Float),
			b_code,
			context.glsl_output_var(EType::Float),
			context.glsl_output_var(EType::Float4)
		);
	});
}

MakeFloat2::MakeFloat2()
{
	r = add_input("r");
	g = add_input("g");

	const auto out = add_output("result");

	out->on_get_type.add_lambda([] { return EType::Float2; });
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!r->is_connected() || !g->is_connected() ||
			r->target()->get_type() != EType::Float ||
			g->target()->get_type() != EType::Float)
			return "";

		const auto r_code = r->target()->get_code(context);
		const auto g_code = g->target()->get_code(context);

		return std::format(
			"vec4 result = vec4(0,0,0,1);\n"
			"{{{}}}\n"
			"result.r = {};\n"
			"{{{}}}\n"
			"result.g = {};\n"
			"{} = result;",
			r_code,
			context.glsl_output_var(EType::Float),
			g_code,
			context.glsl_output_var(EType::Float),
			context.glsl_output_var(EType::Float4)
		);
	});
}

BreakColor::BreakColor()
{
	in = add_input("In");
	const auto r = add_output("R");
	r->on_get_type.add_lambda([] { return EType::Float; });
	r->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!in->is_connected())
			return "";
		const auto in_code = in->target()->get_code(context);
		return std::format(
			"{{{}}}\n"
			"{}.gba = vec3(0,0,1);\n",
			in_code,
			context.glsl_output_var(EType::Float4)
		);
	});

	const auto g = add_output("G");
	g->on_get_type.add_lambda([] { return EType::Float; });
	g->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!in->is_connected())
			return "";
		const auto in_code = in->target()->get_code(context);
		return std::format(
			"{{{}}}\n"
			"{} = {}.g;\n"
			"{}.gba = vec3(0,0,1);\n",
			in_code,
			context.glsl_output_var(EType::Float),
			context.glsl_output_var(EType::Float4),
			context.glsl_output_var(EType::Float4)
		);
	});

	const auto b = add_output("B");
	b->on_get_type.add_lambda([] { return EType::Float; });
	b->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!in->is_connected())
			return "";
		const auto in_code = in->target()->get_code(context);
		return std::format(
			"{{{}}}\n"
			"{} = {}.b;\n"
			"{}.gba = vec3(0,0,1);\n",
			in_code,
			context.glsl_output_var(EType::Float),
			context.glsl_output_var(EType::Float4),
			context.glsl_output_var(EType::Float4)
		);
	});

	const auto a = add_output("A");
	a->on_get_type.add_lambda([] { return EType::Float; });
	a->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!in->is_connected())
			return "";
		const auto in_code = in->target()->get_code(context);
		return std::format(
			"{{{}}}\n"
			"{} = {}.a;\n"
			"{}.gba = vec3(0,0,1);\n",
			in_code,
			context.glsl_output_var(EType::Float),
			context.glsl_output_var(EType::Float4),
			context.glsl_output_var(EType::Float4)
		);
	});
}

REGISTER_NODE(MakeFloat4, NodeInfo("", {"Make Float4", "Append Float4", "Merge To Float 4"}));
REGISTER_NODE(MakeFloat3, NodeInfo("", {"Make Float3", "Append Float3", "Merge To Float 3"}));
REGISTER_NODE(MakeFloat2, NodeInfo("", {"Make Float2", "Append Float2", "Merge To Float 2"}));
REGISTER_NODE(BreakColor, NodeInfo("", {"Break Color", "Break Channels", "Split Channels"}));