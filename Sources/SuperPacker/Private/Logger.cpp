#include "logger.h"

#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

#if _WIN32
#include <windows.h>
HANDLE h_console_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

std::vector<uint8_t> allowed_thread_colors = {
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	18, 19, 20, 22, 23, 24, 26, 27, 28, 30, 31,
	32, 33, 37, 38, 39, 43, 44, 45, 46, 47,
	48, 49, 52, 54, 55, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 70, 71, 73, 74, 75, 76, 78, 79,
	80, 81, 82, 83, 84, 86, 87, 90, 91, 92, 94, 95,
	96, 98, 99, 100, 101, 104, 105, 106, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 120, 121, 122, 123, 125, 127,
	129, 132, 133, 134 ,135 ,138, 139, 140, 142, 143,
	144, 145, 148, 149, 150, 151, 154, 155, 156, 157, 158, 159,
	160, 161, 164, 165, 166, 167, 162, 174, 175,
	176, 177, 178, 179, 180 ,181, 184, 185, 186, 189, 190, 191,
	192, 193, 194, 197, 201, 202, 206, 207,
	208, 209, 210, 211, 214, 215, 220, 222, 223,
	224, 225, 226, 227, 228, 229, 232, 233, 234, 235, 236, 237,
	240, 241, 242, 243 ,244 ,245, 248, 249, 250, 252, 253
};


namespace logger
{
	static std::mutex logger_lock;
	static uint8_t(*get_worker_id_func)() = nullptr;
	


	void log_print(const char* type, int color, const std::string& message, const char* function, size_t line,
		const char* file)
	{
		std::lock_guard<std::mutex> lock(logger_lock);


		struct tm time_str;
		static char time_buffer[80];
		time_t now = time(0);
		localtime_s(&time_str, &now);
		//strftime(time_buffer, sizeof(time_buffer), "%d/%b/%Y %X", &time_str);
		strftime(time_buffer, sizeof(time_buffer), "%X", &time_str);

#if _WIN32
		SetConsoleTextAttribute(h_console_out, color);
#endif
		std::cout << log_format("[%s  ", time_buffer);

		uint8_t worker_id = static_cast<uint8_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
		std::string worker_id_str = log_format("~%x", std::this_thread::get_id());
		if (get_worker_id_func && get_worker_id_func() != 255)
		{
			worker_id_str = log_format("#W%d", get_worker_id_func());
			worker_id = get_worker_id_func();

#if _WIN32
			if (get_worker_id_func && worker_id != 255) SetConsoleTextAttribute(h_console_out, allowed_thread_colors[worker_id % allowed_thread_colors.size()]);
#endif
		}
		else
		{
#if _WIN32
			SetConsoleTextAttribute(h_console_out, CONSOLE_ASSERT);
#endif
		}
				
		std::cout << log_format("%s", worker_id_str.c_str());
#if _WIN32
		SetConsoleTextAttribute(h_console_out, color);
#endif
		if (function) std::cout << log_format("] [%s] % s::% d : %s", type, function, line, message.c_str());
		else std::cout << log_format("] [%s] : %s", type, message.c_str());

		if (file) std::cout << log_format("\n\t=>%s", file);
		
		std::cout << std::endl;
#if _WIN32
		SetConsoleTextAttribute(h_console_out, CONSOLE_DEFAULT);
#endif
	}

	void set_get_worker_func(uint8_t(*getter)())
	{
		get_worker_id_func = getter;
	}
}
