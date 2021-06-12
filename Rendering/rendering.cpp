// Rendering.cpp : Defines the entry point for the application.
//
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <iostream>
#include <memory>
#include <vector>

#include "window.hpp"
#include "vulkan_app.hpp"


using namespace std::string_literals;


void setup_logger()
{	
#ifndef NDEBUG
	spdlog::level::level_enum log_level = spdlog::level::debug;
#else
	spdlog::level::level_enum log_level = spdlog::level::info;
#endif

	std::string log_pattern = "[%H:%M:%S][%^%l%$] %v";

	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(log_level);
	console_sink->set_pattern(log_pattern);

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
	file_sink->set_level(log_level);
	file_sink->set_pattern(log_pattern);

	std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks{ console_sink, file_sink };
	auto logger = std::make_shared<spdlog::logger>("default"s, sinks.begin(), sinks.end());
	logger->set_level(log_level);
	spdlog::set_default_logger(logger);
}

int main()
{
	sys::Window::initializeSystem();
	try
	{
		setup_logger();


		auto window = std::make_shared<sys::Window>(800, 600, "vulkan");

		gfx::VulkanApp vk_app(window);

		while (!window->shouldClose())
		{
			sys::Window::pollEvents();
		}

	}
	catch (std::exception& err)
	{
		//spdlog::critical("{}", err.what());
	}
	sys::Window::terminateSystem();
}
