#pragma once
#include <string>
#include <vector>

enum class ELogType
{
	Debug,
	Info,
	Warning,
	Error
};

class Log
{
public:
	Log() = default;

	Log(ELogType in_type, std::string in_message) : type(in_type), message(std::move(in_message))
	{
	}

	ELogType type = ELogType::Info;
	std::string message;
};

class Logger
{
public:
	Logger();

	void add_frame_log(const Log& log);
	void add_persistent_log(const Log& log);
	void new_frame();
	void set_max_logs(size_t count);
	void print_persistent_logs() const;
	void print_frame_logs() const;

	static Logger& get();

	void display();

	[[nodiscard]] float get_display_height() const { return display_height; }

private:
	std::vector<Log> persistent_logs;
	std::vector<Log> frame_logs;

	size_t persistent_first = 0;
	size_t persistent_count = 0;

	bool display_grow = false;
	float display_height = 18;
};
