#include "node_maths.h"

NodeAdd::NodeAdd() : Node("add")
{
	a = add_input("a");
	b = add_input("b");
	auto out = add_output("result");
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
		{
			if (!*a || !*b || a->target()->on_get_type.execute() != b->target()->on_get_type.execute())
				return "";

			auto a_code = a->target()->on_get_code.execute(context);
			auto b_code = b->target()->on_get_code.execute(context);

			const auto out_type = a->target()->on_get_type.execute();
			const auto output_var = context.glsl_output_var(out_type);
			const auto storage_var = context.generate_name();
			return std::format(
				"{} {};\n" // storage
				"{{\n{}\n}}\n{} = {};\n" // a
				"{{\n{}\n}}\n{} += {};\n" // b
				"{} = {};\n", // result
				context.glsl_type(out_type), storage_var,
				a_code, storage_var, output_var,
				b_code, storage_var, output_var,
				output_var, storage_var
			);
		});

	out->on_get_type.add_lambda([&]()
	{
		return a->target() ? a->target()->on_get_type.execute() : b->target() ? b->target()->on_get_type.execute() : EType::Undefined;
	});
}

void NodeAdd::display()
{
}

NodeMult::NodeMult() : Node("mult")
{
	a = add_input("a");
	b = add_input("b");
	const auto out = add_output("result");
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!*a || !*b || a->target()->on_get_type.execute() != b->target()->on_get_type.execute())
			return "";

		auto a_code = a->target()->on_get_code.execute(context);
		auto b_code = b->target()->on_get_code.execute(context);

		const auto out_type = a->target()->on_get_type.execute();
		const auto output_var = context.glsl_output_var(out_type);
		const auto storage_var = context.generate_name();
		return std::format(
			"{} {};\n" // storage
			"{{\n{}\n}}\n{} = {};\n" // a
			"{{\n{}\n}}\n{} *= {};\n" // b
			"{} = {};\n", // result
			context.glsl_type(out_type), storage_var,
			a_code, storage_var, output_var,
			b_code, storage_var, output_var,
			output_var, storage_var
		);
	});

	out->on_get_type.add_lambda([&]()
		{
			return a->target() ? a->target()->on_get_type.execute() : b->target() ? b->target()->on_get_type.execute() : EType::Undefined;
		});
}

void NodeMult::display()
{
}
