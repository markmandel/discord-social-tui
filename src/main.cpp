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
#include <cstdlib>
#include <iostream>
#include <optional>
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

// Show usage information
void PrintUsage(const std::string& program_name) {
  std::cerr << "Usage: " << program_name << " --application-id=YOUR_APP_ID"
            << '\n';
  std::cerr << "   or: " << program_name << " -a YOUR_APP_ID" << '\n';
  std::cerr << '\n';
  std::cerr << "Environment Variables:" << '\n';
  std::cerr << "   DISCORD_APPLICATION_ID: Discord application ID" << '\n';
}

int main(int argc, char* argv[]) {
  // Convert C-style arguments to a vector
  std::vector<std::string> args(argv, argv + argc);

  // Parse application ID from command line or environment
  auto application_id = ParseApplicationId(args);

  // Check if application ID is provided
  if (!application_id) {
    std::cerr << "Error: Discord Application ID is required." << '\n';
    PrintUsage(args[0]);
    return EXIT_FAILURE;
  }

  // Create Discord client
  auto client = std::make_shared<discordpp::Client>();

  // Create and run application
  discord_social_tui::App app(*application_id, client);
  return app.Run();
}