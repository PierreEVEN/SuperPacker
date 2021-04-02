#pragma once
#include <memory>
#include <string>

#if _DEBUG
#define logger_log(format, ...) logger::log_print("I", logger::ConsoleColor::CONSOLE_DISPLAY, logger::log_format(format, __VA_ARGS__), ##__FUNCTION__, __LINE__)
#define logger_validate(format, ...) logger::log_print("V", logger::ConsoleColor::CONSOLE_VALIDATE, logger::log_format(format, __VA_ARGS__), ##__FUNCTION__, __LINE__)
#define logger_warning(format, ...) logger::log_print("W", logger::ConsoleColor::CONSOLE_WARNING, logger::log_format(format, __VA_ARGS__), ##__FUNCTION__, __LINE__)
#define logger_error(format, ...) logger::log_print("E", logger::ConsoleColor::CONSOLE_FAIL, logger::log_format(format, __VA_ARGS__), ##__FUNCTION__, __LINE__, ##__FILE__)
#define logger_fail(format, ...) { logger::log_print("F", logger::ConsoleColor::CONSOLE_ASSERT, logger::log_format(format, __VA_ARGS__), ##__FUNCTION__, __LINE__, ##__FILE__); __debugbreak(); exit(EXIT_FAILURE); }
#else
#define logger_log(format, ...) logger::log_print("I", logger::ConsoleColor::CONSOLE_DISPLAY, logger::log_format(format, __VA_ARGS__))
#define logger_validate(format, ...) logger::log_print("V", logger::ConsoleColor::CONSOLE_VALIDATE, logger::log_format(format, __VA_ARGS__))
#define logger_warning(format, ...) logger::log_print("W", logger::ConsoleColor::CONSOLE_WARNING, logger::log_format(format, __VA_ARGS__))
#define logger_error(format, ...) logger::log_print("E", logger::ConsoleColor::CONSOLE_FAIL, logger::log_format(format, __VA_ARGS__))
#define logger_fail(format, ...) { logger::log_print("F", logger::ConsoleColor::CONSOLE_ASSERT, logger::log_format(format, __VA_ARGS__), ##__FUNCTION__, __LINE__, ##__FILE__); exit(EXIT_FAILURE); }
#endif


namespace logger
{
	enum ConsoleColor {
		CONSOLE_FG_COLOR_BLACK = 0,
		CONSOLE_FG_COLOR_BLUE = 1,
		CONSOLE_FG_COLOR_GREEN = 2,
		CONSOLE_FG_COLOR_RED = 4,
		CONSOLE_FG_COLOR_LIGHT = 8,
		CONSOLE_FG_COLOR_LIGHT_RED = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_LIGHT,
		CONSOLE_FG_COLOR_WHITE = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_GREEN | CONSOLE_FG_COLOR_BLUE,
		CONSOLE_FG_COLOR_ORANGE = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_GREEN,
		CONSOLE_FG_COLOR_VIOLET = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_BLUE,
		CONSOLE_FG_COLOR_LIGHT_BLUE = CONSOLE_FG_COLOR_GREEN | CONSOLE_FG_COLOR_BLUE,
		CONSOLE_FG_COLOR_CYAN = CONSOLE_FG_COLOR_GREEN | CONSOLE_FG_COLOR_BLUE | CONSOLE_FG_COLOR_LIGHT,
		CONSOLE_BG_COLOR_BLUE = 16,
		CONSOLE_BG_COLOR_GREEN = 32,
		CONSOLE_BG_COLOR_RED = 64,
		CONSOLE_BG_COLOR_LIGHT = 128,
		CONSOLE_DEFAULT = CONSOLE_FG_COLOR_WHITE,
		CONSOLE_VALIDATE = CONSOLE_FG_COLOR_GREEN,
		CONSOLE_DISPLAY = CONSOLE_FG_COLOR_LIGHT_BLUE,
		CONSOLE_WARNING = CONSOLE_FG_COLOR_ORANGE,
		CONSOLE_FAIL = CONSOLE_FG_COLOR_LIGHT_RED,
		CONSOLE_ASSERT = CONSOLE_FG_COLOR_VIOLET | CONSOLE_BG_COLOR_RED | CONSOLE_BG_COLOR_GREEN | CONSOLE_BG_COLOR_LIGHT
	};
	
	template<typename... Params>
	std::string log_format(const char* format, Params... args) {
		const int size = snprintf(nullptr, 0, format, args...) + 1;
		if (size <= 0) return format;
		const std::unique_ptr<char[]> buffer(new char[size]);
		snprintf(buffer.get(), size, format, args ...);
		return std::string(buffer.get());
	}
	
	void log_print(const char* type, int color, const std::string& message, const char* function = nullptr, size_t line = 0, const char* file = nullptr);

	template<typename... Params>
	void log(const char* format, Params... args)	{
		log_print("I", CONSOLE_DISPLAY, logger::log_format(format, std::forward<Params>(args)...));
	}
	
	template<typename... Params>
	void validate(const char* format, Params... args) {
		log_print("V", CONSOLE_VALIDATE, logger::log_format(format, std::forward<Params>(args)...));
	}
	
	template<typename... Params>
	void warning(const char* format, Params... args) {
		log_print("W", CONSOLE_WARNING, logger::log_format(format, std::forward<Params>(args)...));
	}
		
	template<typename... Params>
	void error(const char* format, Params... args) {
		log_print("E", CONSOLE_FAIL, logger::log_format(format, std::forward<Params>(args)...));
	}

	template<typename... Params>
	void fail(const char* format, Params... args) {
		log_print("F", CONSOLE_ASSERT, logger::log_format(format, std::forward<Params>(args)...));
#if _DEBUG
		__debugbreak();
#endif
		exit(EXIT_FAILURE);
	}


	void set_get_worker_func(uint8_t(*getter)());
}
