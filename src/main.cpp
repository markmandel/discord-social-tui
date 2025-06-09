// Copyright 2025 Mark Mandel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define DISCORDPP_IMPLEMENTATION
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

#include "app/app.hpp"
#include "discordpp.h"

// Get environment variable
std::optional<std::string> GetEnv(const std::string& var_name) {
  const char* value = std::getenv(var_name.c_str());
  if (value != nullptr) {
    return std::string(value);
  }
  return std::nullopt;
}

// Parse application ID from command line arguments and environment variables
std::optional<std::string> ParseApplicationId(
    const std::vector<std::string>& args) {
  static constexpr size_t PREFIX_LENGTH = 17;  // Length of "--application-id="

  // Check command-line arguments first
  // Format: --application-id=YOUR_APP_ID or -a YOUR_APP_ID
  for (size_t i = 1; i < args.size(); ++i) {
    const std::string& arg = args[i];

    if (arg.starts_with("--application-id=")) {
      // --application-id=value format
      return arg.substr(PREFIX_LENGTH);
    }
    if (arg == "--application-id" || arg == "-a") {
      // --application-id value or -a value format
      if (i + 1 < args.size()) {
        ++i;             // Move to next argument
        return args[i];  // Return the next argument as the value
      }
    }
  }

  // Check environment variable if no command-line argument
  return GetEnv("DISCORD_APPLICATION_ID");
}

// Parse log file name from command line arguments
std::string ParseLogFileName(const std::vector<std::string>& args) {
  static constexpr size_t PREFIX_LENGTH = 11;  // Length of "--log-file="

  // Default log file name
  std::string log_file_name = "log";

  // Check command-line arguments
  // Format: --log-file=FILE_NAME or -l FILE_NAME
  for (size_t i = 1; i < args.size(); ++i) {
    const std::string& arg = args[i];

    if (arg.starts_with("--log-file=")) {
      // --log-file=value format
      return arg.substr(PREFIX_LENGTH);
    }
    if (arg == "--log-file" || arg == "-l") {
      // --log-file value or -l value format
      if (i + 1 < args.size()) {
        ++i;             // Move to next argument
        return args[i];  // Return the next argument as the value
      }
    }
  }

  // Return default if not specified
  return log_file_name;
}

// Show usage information
void PrintUsage(const std::string& program_name) {
  std::cerr << "Usage: " << program_name << " --application-id=YOUR_APP_ID"
            << " [--log-file=FILE_NAME]" << '\n';
  std::cerr << "   or: " << program_name << " -a YOUR_APP_ID"
            << " [-l FILE_NAME]" << '\n';
  std::cerr << '\n';
  std::cerr << "Options:" << '\n';
  std::cerr
      << "   --application-id, -a  <ID>    Discord application ID (required)"
      << '\n';
  std::cerr << "   --log-file, -l        <FILE>  Log file name (default: 'log')"
            << '\n';
  std::cerr << '\n';
  std::cerr << "Environment Variables:" << '\n';
  std::cerr << "   DISCORD_APPLICATION_ID: Discord application ID" << '\n';
}

bool ConfigureLogger(const std::string& log_file_name) {
  constexpr int FLUSH_INTERVAL = 2;

  try {
    // Create a file sink with the provided file name
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        log_file_name, true);

    // Create logger with file sink
    auto logger = std::make_shared<spdlog::logger>("logger", file_sink);

    // Set as default logger
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(FLUSH_INTERVAL));

    // Set JSON pattern with required fields
    spdlog::set_pattern(
        R"({"level":"%l","message":"%v","time":"%Y-%m-%d %H:%M:%S.%e","source":"%g"})");

    // Sets log level based on SPDLOG_LEVEL environment variable
    spdlog::cfg::load_env_levels();

    // Log initialization message
    SPDLOG_INFO("Logging initialized with file: {}", log_file_name);
    return true;
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Log initialization failed: " << ex.what() << '\n';
    return false;
  }
}

// Make string safe for JSON by escaping special characters
std::string MakeJsonSafe(const std::string& input) {
  std::string result = input;

  // Remove CR and LF
  result.erase(std::ranges::remove(result, '\n').begin(), result.end());
  result.erase(std::ranges::remove(result, '\r').begin(), result.end());

  // Replace backslashes with double backslashes
  result = std::regex_replace(result, std::regex("\\\\"), "\\\\");

  // Replace double quotes with escaped double quotes
  result = std::regex_replace(result, std::regex("\""), "\\\"");

  // Replace control characters
  result = std::regex_replace(result, std::regex("\b"), "\\b");
  result = std::regex_replace(result, std::regex("\f"), "\\f");
  result = std::regex_replace(result, std::regex("\t"), "\\t");

  return result;
}

void StartDiscordLogging(const std::shared_ptr<discordpp::Client>& client) {
  client->AddLogCallback(
      [](const std::string& message,
         const discordpp::LoggingSeverity severity) {
        // Make message safe for JSON
        std::string json_safe_message = MakeJsonSafe(message);

        switch (severity) {
          case discordpp::LoggingSeverity::Verbose:
            SPDLOG_LOGGER_CALL(spdlog::default_logger(), spdlog::level::trace,
                               json_safe_message);
            break;
          case discordpp::LoggingSeverity::Info:
            SPDLOG_LOGGER_CALL(spdlog::default_logger(), spdlog::level::info,
                               json_safe_message);
            break;
          case discordpp::LoggingSeverity::Warning:
            SPDLOG_LOGGER_CALL(spdlog::default_logger(), spdlog::level::warn,
                               json_safe_message);
            break;
          case discordpp::LoggingSeverity::Error:
            SPDLOG_LOGGER_CALL(spdlog::default_logger(), spdlog::level::err,
                               json_safe_message);
            break;
          case discordpp::LoggingSeverity::None:
            break;
        }
      },
      discordpp::LoggingSeverity::Info);
}

int main(const int argc, char* argv[]) {
  // Convert C-style arguments to a vector
  const std::vector<std::string> args(argv, argv + argc);

  // Parse log file name from command line
  const std::string log_file_name = ParseLogFileName(args);

  // Set up logging first with the specified log file name
  if (!ConfigureLogger(log_file_name)) {
    return EXIT_FAILURE;
  }

  // Parse application ID from command line or environment
  const auto application_id = ParseApplicationId(args);

  // Check if application ID is provided
  if (!application_id) {
    std::cerr << "Error: Discord Application ID is required." << '\n';
    PrintUsage(args[0]);
    return EXIT_FAILURE;
  }

  // Log the application ID
  SPDLOG_INFO("Starting with application ID: {}", *application_id);

  // Create Discord client
  const auto client = std::make_shared<discordpp::Client>();
  StartDiscordLogging(client);

  // Create and run application
  discord_social_tui::App app(std::stoull(*application_id), client);
  return app.Run();
}