#pragma once
#include <imgui.h>
#include <string>
#include <utility>

#include "event_manager.h"
#include "packer/code.h"
#include "packer/types.h"

DECLARE_DELEGATE_SINGLECAST_RETURN(EventGetOutputType, EType);
DECLARE_DELEGATE_SINGLECAST_RETURN(EventGetCode, std::string, CodeContext&);

class Node;

class Pin
{
public:
	Pin(Node* parent, std::string name) : pin_name(std::move(name)), parent_node(parent)
	{
	}

	[[nodiscard]] const std::string& get_pin_name() const { return pin_name; }
	[[nodiscard]] Node* get_parent() const { return parent_node; }
	[[nodiscard]] const ImVec2& get_display_pos() const { return graph_display_pos; }

	[[nodiscard]] virtual bool is_connected() const = 0;

	void update_display_pos(const ImVec2& new_pos) { graph_display_pos = new_pos; }
private:
	const std::string pin_name;
	Node* parent_node;
	ImVec2 graph_display_pos;
};

class OutputPin : public Pin
{
	friend class InputPin;
public:
	OutputPin(Node* in_parent_node, std::string in_name) : Pin(in_parent_node, std::move(in_name)) { }

	// Events
	EventGetOutputType on_get_type;
	EventGetCode on_get_code;

	[[nodiscard]] std::string get_code(CodeContext& code_context);
	[[nodiscard]] EType get_type() { return on_get_type.execute(); }
	[[nodiscard]] bool is_connected() const override { return !link_destinations.empty(); }

	void mark_dirty() { code = nullptr; }
	void break_links() const;

	operator bool() const { return link_destinations.empty(); }

private:
	std::shared_ptr<std::string> code = nullptr;
	std::vector<InputPin*> link_destinations;
};

class InputPin : public Pin
{
public:
	InputPin(Node* in_owner, std::string in_name) : Pin(in_owner, std::move(in_name))
	{
	}

	[[nodiscard]] const std::shared_ptr<OutputPin>& target() const { return link_target; }
	[[nodiscard]] bool is_connected() const override { return link_target != nullptr; }

	void link_to(const std::shared_ptr<OutputPin>& output);
	
private:
	std::shared_ptr<OutputPin> link_target;
};
