
#include "pin.h"

#include "node.h"

std::string OutputPin::get_code(CodeContext& code_context)
{
	if (!code)
	{
		code = std::make_shared<std::string>(on_get_code.execute(code_context));
	}
	return *code;
}

void OutputPin::break_links() const
{
	while (is_connected())
		link_destinations[0]->link_to(nullptr);
}

void InputPin::link_to(const std::shared_ptr<OutputPin>& output)
{
	if (link_target != output)
	{
		if (link_target)
		{
			link_target->link_destinations.erase(std::find(link_target->link_destinations.begin(),
				link_target->link_destinations.end(), this));
			link_target->get_parent()->on_update.clear_object(get_parent());
		}

		link_target = output;
		if (link_target)
		{
			link_target->link_destinations.emplace_back(this);
			link_target->get_parent()->on_update.add_object(get_parent(), &Node::mark_dirty);
		}
		get_parent()->mark_dirty();
	}
}
