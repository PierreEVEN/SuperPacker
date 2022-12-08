#include "node_maths.h"

#include <iostream>

NodeAdd::NodeAdd()
{
	a = add_input("a");
	b = add_input("b");
	auto out = add_output("result");
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!*a || !*b)
			return "";

		const auto a_type = a->target()->on_get_type.execute();
		const auto b_type = b->target()->on_get_type.execute();

		if (a_type != b_type && !(a_type == EType::Float || b_type == EType::Float))
			return "";

		const auto a_code = a->target()->on_get_code.execute(context);
		const auto b_code = b->target()->on_get_code.execute(context);

		const auto code_big = b_type == EType::Float ? a_code : b_code;
		const auto code_small = b_type == EType::Float ? b_code : a_code;
		const auto big_type = b_type == EType::Float ? a_type : b_type;
		const auto small_type = b_type == EType::Float ? b_type : a_type;

		const auto output_var = context.glsl_output_var(big_type);
		const auto storage_var = context.generate_name();
		return std::format(
			"{} {};\n"
			"{{\n"
			"\t{}\n"
			"}}\n"
			"{} = {};\n"
			"{{\n"
			"\t{}\n"
			"}}\n"
			"{} += {}({});\n"
			"{} = {};\n",
			context.glsl_type(big_type), storage_var,
			code_big,
			storage_var, context.glsl_output_var(big_type),
			code_small,
			storage_var, context.glsl_type(big_type), context.glsl_output_var(small_type),
			output_var, storage_var
		);
	});

	out->on_get_type.add_lambda([&]()
	{
		if (!*a || !*b)
			return EType::Undefined;

		const auto a_type = a->target()->on_get_type.execute();
		const auto b_type = b->target()->on_get_type.execute();
		const auto big_type = b_type == EType::Float ? a_type : b_type;
		return big_type;
	});
}

NodeMult::NodeMult()
{
	a = add_input("a");
	b = add_input("b");
	const auto out = add_output("result");
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!*a || !*b)
			return "";

		const auto a_type = a->target()->on_get_type.execute();
		const auto b_type = b->target()->on_get_type.execute();

		if (a_type != b_type && !(a_type == EType::Float || b_type == EType::Float))
			return "";

		const auto a_code = a->target()->on_get_code.execute(context);
		const auto b_code = b->target()->on_get_code.execute(context);

		const auto code_big = b_type == EType::Float ? a_code : b_code;
		const auto code_small = b_type == EType::Float ? b_code : a_code;
		const auto big_type = b_type == EType::Float ? a_type : b_type;
		const auto small_type = b_type == EType::Float ? b_type : a_type;

		const auto output_var = context.glsl_output_var(big_type);
		const auto storage_var = context.generate_name();
		return std::format(
			"{} {};\n"
			"{{\n"
			"\t{}\n"
			"}}\n"
			"{} = {};\n"
			"{{\n"
			"\t{}\n"
			"}}\n"
			"{} *= {}({});\n"
			"{} = {};\n",
			context.glsl_type(big_type), storage_var,
			code_big,
			storage_var, context.glsl_output_var(big_type),
			code_small,
			storage_var, context.glsl_type(big_type), context.glsl_output_var(small_type),
			output_var, storage_var
		);
	});

	out->on_get_type.add_lambda([&]()
	{
			if (!*a || !*b)
				return EType::Undefined;

			const auto a_type = a->target()->on_get_type.execute();
			const auto b_type = b->target()->on_get_type.execute();
			const auto big_type = b_type == EType::Float ? a_type : b_type;
			return big_type;
	});
}

REGISTER_NODE(NodeAdd, NodeInfo("", {"Add", "+"}));
REGISTER_NODE(NodeMult, NodeInfo("", {"Mult", "*"}));