// Rendering.cpp : Defines the entry point for the application.
//
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <iostream>
#include <memory>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iomanip>

#include "window.hpp"
#include "config/logger_config.hpp"



namespace fs = std::filesystem;
using namespace std::string_literals;


config::LoggerConfig setup_logger()
{	

	const std::string config_filename = "logger_config.json";
	config::LoggerConfig config;

	try
	{
		config.loadFile(config_filename);
	}
	catch (std::exception& e)
	{

		using json = nlohmann::json;
		spdlog::warn("Unable to load logger configuration. '{}'. Using default configuration.", e.what());
		
		json default_json = config::LoggerConfig::getDefaultJson();

		if (!fs::exists(config_filename))
		{
			spdlog::warn("Logger config file doesn't exist. Creating.");
			std::ofstream oss(config_filename);
			oss << std::setw(4) << default_json;
		}

		config.load(default_json);
	}

	return config;
}

int main()
{
	sys::Window::initializeSystem();

	

	try
	{
		config::LoggerConfig logger_config = setup_logger();

		auto window = std::make_shared<sys::Window>(800, 600, "vulkan");

		while (!window->shouldClose())
		{
			sys::Window::pollEvents();
		}

	}
	catch (std::exception& err)
	{
		spdlog::critical("{}", err.what());
	}
	sys::Window::terminateSystem();
}
