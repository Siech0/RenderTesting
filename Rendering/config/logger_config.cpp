#include "logger_config.hpp"

#include <fmt/format.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <exception>
#include <fstream>
#include <utility>
#include <limits>
#include <cstdint>

namespace
{
	spdlog::level::level_enum string_to_level(std::string_view text)
	{
		if (text == "trace")
		{
			return spdlog::level::trace;
		}
		else if (text == "debug")
		{
			return spdlog::level::debug;
		}
		else if (text == "info")
		{
			return spdlog::level::info;
		}
		else if (text == "warn")
		{
			return spdlog::level::warn;
		}
		else if (text == "err")
		{
			return spdlog::level::err;
		}
		else if (text == "critical")
		{
			return spdlog::level::critical;
		}
		else if (text == "off")
		{
			return spdlog::level::off;
		}
		else
		{
			throw std::runtime_error(fmt::format("Invalid log level string '{}'", text));
		}
	}

	std::string level_to_string(spdlog::level::level_enum level)
	{
		switch (level)
		{
		case spdlog::level::trace:
			return "trace";
		case spdlog::level::debug:
			return "debug";
		case spdlog::level::info:
			return "info";
		case spdlog::level::warn:
			return "warn";
		case spdlog::level::err:
			return "err";
		case spdlog::level::critical:
			return "critical";
		case spdlog::level::off:
			return "off";
		default:
			throw std::runtime_error(fmt::format("Invalid log level '{}'", level));
		}
	}
}



namespace config
{

	LoggerConfig::LoggerConfig(std::string_view data)
	{
		load(data);
	}

	LoggerConfig::LoggerConfig(std::istream& iss)
	{
		load(iss);
	}

	void LoggerConfig::load(std::string_view data)
	{
		json json;
		json = data;
		load(json);
	}

	void LoggerConfig::load(std::istream& iss)
	{
		json json;
		iss >> json;
		load(json);
	}

	void LoggerConfig::load(json& json)
	{
		// Load default pattern
		if (json.contains("default_pattern"))
		{
			json.at("default_pattern").get_to(m_defaultPattern);
		}
		
		if (json.contains("default_level"))
		{
			std::string level_string;
			json.at("default_level").get_to(level_string);
			m_defaultLevel = string_to_level(level_string);
		}

		// Load named patterns
		if (json.contains("patterns"))
		{
			auto pattern_config = json.at("patterns");
			for (auto& [name, pattern] : pattern_config.items())
			{
				m_patternMap.emplace(name, pattern);
			}
		}
		
		// Load named sinks
		if (json.contains("sinks"))
		{
			auto sinks_config = json.at("sinks");
			for (auto& [sink_name, sink_config] : sinks_config.items())
			{
				auto sink = readSink(sink_config, sink_name);
				m_sinkMap.emplace(sink_name, sink);
			}
		}

		// Load named loggers
		if (json.contains("loggers"))
		{
			auto loggers_config = json.at("loggers");
			for (auto& [logger_name, logger_config] : loggers_config.items())
			{
				
				auto logger = readLogger(logger_config, logger_name);
				spdlog::register_logger(logger);
				m_loggerMap.emplace(logger_name, logger);
			}
		}

		// Load default logger
		if (json.contains("default_logger"))
		{
			std::string logger_name;
			json.at("default_logger").get_to(logger_name);
			if (auto it = m_loggerMap.find(logger_name); it != m_loggerMap.cend())
			{
				spdlog::set_default_logger(it->second);
			}
			else
			{
				throw std::runtime_error(fmt::format("Unable to set default logger to '{}', logger not found.", logger_name));
			}
		}
	}


	std::shared_ptr<spdlog::sinks::sink> LoggerConfig::readSink(json& json, std::string_view sink_name)
	{
		std::string type_str;
		std::shared_ptr<spdlog::sinks::sink> sink;

		// Not optional, error if not present
		json.at("type").get_to(type_str);

		// Enum class used to make parsing simpler
		enum class SinkType
		{
			eStdoutSinkSt,
			eStdoutSinkMt,
			eStderrSinkSt,
			eStderrSinkMt,
			eStdoutColorSinkSt,
			eStdoutColorSinkMt,
			eStderrColorSinkSt,
			eStderrColorSinkMt,
			eBasicFileSinkSt,
			eBasicFileSinkMt,
			eDailyFileSinkSt,
			eDailyFileSinkMt,
			eRotatingFileSinkSt,
			eRotatingFileSinkMt
		} sink_type;

		if (type_str == "stdout_sink_st")
			sink_type = SinkType::eStdoutSinkSt;
		else if (type_str == "stdout_sink_mt")
			sink_type = SinkType::eStdoutSinkMt;
		else if (type_str == "stderr_sink_st")
			sink_type = SinkType::eStderrSinkSt;
		else if (type_str == "stderr_sink_mt")
			sink_type = SinkType::eStderrSinkMt;
		else if (type_str == "stdout_color_sink_st")
			sink_type = SinkType::eStdoutColorSinkSt;
		else if (type_str == "stdout_color_sink_mt")
			sink_type = SinkType::eStdoutColorSinkMt;
		else if (type_str == "stderr_color_sink_st")
			sink_type = SinkType::eStderrColorSinkSt;
		else if (type_str == "stderr_color_sink_mt")
			sink_type = SinkType::eStderrColorSinkMt;
		else if (type_str == "basic_file_sink_st")
			sink_type = SinkType::eBasicFileSinkSt;
		else if (type_str == "basic_file_sink_mt")
			sink_type = SinkType::eBasicFileSinkMt;
		else if (type_str == "daily_file_sink_st")
			sink_type = SinkType::eDailyFileSinkSt;
		else if (type_str == "daily_file_sink_mt")
			sink_type = SinkType::eDailyFileSinkMt;
		else if (type_str == "rotating_file_sink_st")
			sink_type = SinkType::eRotatingFileSinkSt;
		else if (type_str == "rotating_file_sink_mt")
			sink_type = SinkType::eRotatingFileSinkMt;
		else
			throw std::runtime_error(fmt::format("Invalid logger sink type: '{}'", type_str));

		if (sink_type == SinkType::eStdoutSinkSt)
			sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
		else if (sink_type == SinkType::eStdoutSinkMt)
			sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
		else if (sink_type == SinkType::eStderrSinkSt)
			sink = std::make_shared<spdlog::sinks::stderr_sink_st>();
		else if (sink_type == SinkType::eStderrSinkMt)
			sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
		else if (sink_type == SinkType::eStdoutColorSinkSt)
			sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
		else if (sink_type == SinkType::eStdoutColorSinkMt)
			sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		else if (sink_type == SinkType::eStderrColorSinkSt)
			sink = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
		else if (sink_type == SinkType::eStderrColorSinkMt)
			sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
		else if (sink_type == SinkType::eBasicFileSinkSt || sink_type == SinkType::eBasicFileSinkMt)
		{
			std::string filename = fmt::format("{}.log", sink_name);
			bool truncate = false;

			if (json.contains("filename"))
				json.at("filename").get_to(filename);

			if (json.contains("truncate"))
				json.at("truncate").get_to(truncate);

			if (sink_type == SinkType::eBasicFileSinkSt)
				sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(filename, truncate);
			else if (sink_type == SinkType::eBasicFileSinkMt)
				sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, truncate);
		}
		else if (sink_type == SinkType::eDailyFileSinkSt || sink_type == SinkType::eDailyFileSinkMt)
		{

			std::string base_filename = fmt::format("{}.log", sink_name);
			int rotation_hour = 0;
			bool truncate = false;
			uint16_t max_files = std::numeric_limits<uint16_t>::max();

			if (json.contains("base_filename"))
				json.at("base_filename").get_to(base_filename);
			if (json.contains("truncate"))
				json.at("truncate").get_to(truncate);
			if (json.contains("rotation_hour"))
				json.at("rotation_hour").get_to(rotation_hour);
			if (json.contains("max_files"))
				json.at("max_files").get_to(max_files);

			if (sink_type == SinkType::eDailyFileSinkSt)
				sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(base_filename, rotation_hour, truncate, max_files);
			else if (sink_type == SinkType::eDailyFileSinkMt)
				sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(base_filename, rotation_hour, truncate, max_files);
		}
		else if (sink_type == SinkType::eRotatingFileSinkSt || sink_type == SinkType::eRotatingFileSinkMt)
		{
			std::string base_filename;
			size_t max_size = std::numeric_limits<size_t>::max(), max_files = std::numeric_limits<size_t>::max();
			bool rotate_on_open = false;

			if (json.contains("base_filename"))
				json.at("base_filename").get_to(base_filename);
			if (json.contains("max_size"))
				json.at("max_size").get_to(max_size);
			if (json.contains("max_files"))
				json.at("max_files").get_to(max_files);
			if (json.contains("rotate_on_open"))
				json.at("rotate_on_open").get_to(rotate_on_open);

			if (sink_type == SinkType::eRotatingFileSinkSt)
				sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(base_filename, max_size, max_files, rotate_on_open);
			else if (sink_type == SinkType::eRotatingFileSinkMt)
				sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(base_filename, max_size, max_files, rotate_on_open);
		}
		
		if (json.contains("level"))
		{
			std::string level_str;
			spdlog::level::level_enum level;

			json.at("level").get_to(level_str);
			level = string_to_level(level_str);
			sink->set_level(level);
		}
		else
		{
			sink->set_level(m_defaultLevel);
		}

		if (json.contains("pattern"))
		{
			std::string pattern;
			json.at("pattern").get_to(pattern);

			// Check for named pattern
			if (auto it = m_patternMap.find(pattern); it != m_patternMap.end())
			{
				pattern = it->second;

			}
			sink->set_pattern(pattern);
		}
		else
		{
			sink->set_pattern(m_defaultPattern);
		}

		return sink;

		return std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
	}

	std::shared_ptr<spdlog::logger> LoggerConfig::readLogger(json& json, std::string_view logger_name)
	{
		auto logger = std::make_shared<spdlog::logger>(std::string(logger_name));

		if (json.contains("sinks"))
		{
			for (auto& sink_name : json.at("sinks"))
			{
				if (auto it = m_sinkMap.find(sink_name); it != m_sinkMap.cend())
				{
					logger->sinks().emplace_back(it->second);
				}
				else
				{
					throw std::runtime_error(fmt::format("Unable to find sink with name '{}' referenced by logger '{}'", sink_name, logger_name));
				}
			}
		}

		if (json.contains("level"))
		{
			std::string level_str;
			spdlog::level::level_enum level;

			json.at("level").get_to(level_str);
			level = string_to_level(level_str);
			logger->set_level(level);
		}
		else
		{
			logger->set_level(m_defaultLevel);
		}

		if (json.contains("pattern"))
		{
			std::string pattern = m_defaultPattern;
			json.at("pattern").get_to(pattern);

			// Check for named pattern
			if (auto it = m_patternMap.find(pattern); it != m_patternMap.end())
			{
				pattern = it->second;
			}

			logger->set_pattern(pattern);
		}

		return logger;
	}

	void LoggerConfig::loadFile(fs::path path)
	{
		std::ifstream iss(path);
		if (!iss)
			throw std::runtime_error(fmt::format("Unable to open file '{}'", path.string()));
		load(iss);
	}

	json LoggerConfig::getDefaultJson()
	{
		json json = R"({
			"default_logger": "default_logger",
			"default_pattern": "default_pattern",
			"patterns": {
				 "default_pattern": "[%H:%M:%S][%^%l%$] %v"
			},
			"sinks": {
				"basic_file": {
					"type": "basic_file_sink_mt",
					"filename": "output.log",
					"pattern": "default_pattern",
					"truncate": true
				},
				"vulkan_file": {
					"type": "basic_file_sink_mt",
					"filename": "vulkan.log",
					"pattern": "default_pattern",
					"truncate": true
				},
				"console_stdout": {
					"type": "stdout_color_sink_mt"
				}
			},
			"loggers": {
				"default_logger": {		
					"pattern": "default_pattern",
					"sinks": [
						"console_stdout",
						"basic_file"
					]
				},
				"vulkan": {
					"level": "info",
					"pattern": "default_pattern",
					"sinks": [
						"console_stdout", 
						"vulkan_file"
				]
				}
			}
		})"_json;

#ifndef NDEBUG
		json["default_level"] = "debug"; 
#else
		json["default_level"] = "info";
#endif

		return json;
	}

	std::istream& operator>>(std::istream& iss, LoggerConfig& config)
	{
		config.load(iss);
		return iss;
	}

}