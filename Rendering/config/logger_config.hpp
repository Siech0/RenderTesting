#pragma once


#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <iostream>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

using json = nlohmann::json;

namespace config
{
	class LoggerConfig
	{
		std::unordered_map<std::string, std::string> m_patternMap;
		std::unordered_map<std::string, std::shared_ptr<spdlog::sinks::sink>> m_sinkMap;
		std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> m_loggerMap;

		std::string m_defaultPattern = "[%H:%M:%S][%^%l%$] %v";
		std::shared_ptr<spdlog::logger> m_defaultLogger;
		
#ifndef NDEBUG
		spdlog::level::level_enum m_defaultLevel = spdlog::level::debug;
#else
		spdlog::level::level_enum m_defaultLevel = spdlog::level::info;
#endif


		/// <summary>
		/// Read configuration data for sink given its specific json configuration
		/// NOTE: This should be done after patterns are loaded (including the default pattern)
		///   as m_patternMap and m_defaultPattern are referenced during the construction of the sink
		/// </summary>
		/// <param name="json">Json configuration for a sink object</param>
		/// <returns> A std::pair containing a string containing the name of the sink, and a shared pointer to the sink</returns>
		std::shared_ptr<spdlog::sinks::sink> readSink(json& json, std::string_view sink_name);

		/// <summary>
		/// Read configuration data for a logger given its specific json configuration
		/// NOTE: This should be done after patterns and sinks are loaded (including the default pattern)
		///   as m_patternMap, m_sinkMap, and m_defaultPattern are referenced during the construction of the 
		///   logger
		/// </summary>
		/// <param name="json">Json configuration for a logger object</param>
		/// <returns> A std::pair containing  astirng containing the name of the sink and a shared pointer to the logger</returns>
		std::shared_ptr<spdlog::logger> readLogger(json& json, std::string_view logger_name);

	public:
		LoggerConfig() = default;
		LoggerConfig(const LoggerConfig& other) = default;
		LoggerConfig(LoggerConfig&& other) = default;

		/// <summary>
		///	Construct a logging configuration object via the load(const std::string&) member function
		/// </summary>
		/// <param name="data">json-encoded string containing configuration data</param>
		LoggerConfig(std::string_view data);

		/// <summary>
		///	Construct a logger configuration object via the load(std::istream&) member function
		/// </summary>
		/// <param name="iss">Input stream containing json-encoded configuration data</param>
		LoggerConfig(std::istream& iss);

		/// <summary>
		/// Load logger configuration stored as a json-encoded string.
		/// </summary>
		/// <param name="data"></param>
		void load(std::string_view data);

		/// <summary>
		/// Load logger configuration stored as a json-encoded string in an input stream
		/// </summary>
		/// <param name="iss">Input stream to read json-encoded data from</param>
		void load(std::istream& iss);

		/// <summary>
		/// Load logger configuration stored as a nlohmann-json json object
		/// </summary>
		/// <param name="json">nlohmann-json json object containing configuration data</param>
		void load(json& json);

		/// <summary>
		/// Load logger configuration stored in a file at the given path
		/// </summary>
		/// <param name="path">Path to file containing json-encoded configuration data</param>
		void loadFile(fs::path path);

		/// <summary>
		/// Returns a default configuration json object
		/// </summary>
		/// <returns>The defaullt configuration json object</returns>
		static json getDefaultJson();

		/// <summary>
		/// Read json-encoded configuration data from an input stream
		/// </summary>
		/// <param name="iss">Input stream to read from</param>
		/// <param name="config">Configuration data to write to</param>
		/// <returns>Input stream read from</returns>
		friend std::istream& operator>>(std::istream& iss, LoggerConfig& config);

	};




}