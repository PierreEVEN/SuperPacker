#include "logger.h"

#include <imgui.h>
#include <iostream>
#include <memory>
#include <ostream>
#include <imgui_operators.h>

#include "gfx.h"

Logger::Logger()
{
	persistent_logs.resize(1000);
}

void Logger::add_frame_log(const Log& log)
{
	frame_logs.emplace_back(log);
}

void Logger::add_persistent_log(const Log& log)
{
	persistent_logs[persistent_first] = log;
	persistent_first = (persistent_first + 1) % persistent_logs.size();
	persistent_count++;
}

void Logger::new_frame()
{
	frame_logs.clear();
}

void Logger::set_max_logs(size_t count)
{
	persistent_logs.resize(count);
}

void Logger::print_persistent_logs() const
{
	for (int64_t i = persistent_count; i > 0; --i)
	{
		const auto& log = persistent_logs[((persistent_first - i) + persistent_logs.size()) % persistent_logs.size()];
		ImGui::Text("%s", log.message.c_str());
	}
}

void Logger::print_frame_logs() const
{
	for (const auto& log : frame_logs)
	{
		ImGui::Text("%s", log.message.c_str());
	}
}

static std::unique_ptr<Logger> logger = nullptr;

Logger& Logger::get()
{
	if (!logger)
		logger = std::make_unique<Logger>();

	return *logger;
}

void Logger::display()
{
	display_height = std::lerp(display_height, display_grow ? 200.f : 18.f, Gfx::get().get_delta_second() * 20.f);
	ImGui::SetCursorPos({0, ImGui::GetWindowSize().y - display_height});
	if (ImGui::BeginChild("logger", {ImGui::GetWindowSize().x, display_height}))
	{
		if (ImGui::BeginChild("logs"))
		{
			ImGui::Dummy({10, 0});
			ImGui::SameLine();
			ImGui::BeginGroup();
			print_frame_logs();
			print_persistent_logs();
			ImGui::EndGroup();
			if (!display_grow)
				ImGui::SetScrollHereY(1);
		}
		ImGui::EndChild();
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			display_grow = !display_grow;
		}
	}
	new_frame();

	ImGui::EndChild();
}
