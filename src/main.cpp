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

#include "app/app.hpp"
#include "discordpp.h"

// Get environment variable
std::optional<std::string> GetEnv(const std::string& var_name) {
  const char* value = std::getenv(var_name.c_str());
  if (value) {
    return std::string(value);
  }
  return std::nullopt;
}

// Parse application ID from command line arguments and environment variables
std::optional<std::string> ParseApplicationId(int argc, char* argv[]) {
  // Check command-line arguments first
  // Format: --application-id=YOUR_APP_ID or -a YOUR_APP_ID
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg.find("--application-id=") == 0) {
      // --application-id=value format
      return arg.substr(17);
    } else if (arg == "--application-id" || arg == "-a") {
      // --application-id value or -a value format
      if (i + 1 < argc) {
        return std::string(argv[++i]);  // Move to next argument for the value
      }
    }
  }

  // Check environment variable if no command-line argument
  return GetEnv("DISCORD_APPLICATION_ID");
}

// Show usage information
void PrintUsage(const std::string& program_name) {
  std::cerr << "Usage: " << program_name << " --application-id=YOUR_APP_ID"
            << std::endl;
  std::cerr << "   or: " << program_name << " -a YOUR_APP_ID" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Environment Variables:" << std::endl;
  std::cerr << "   DISCORD_APPLICATION_ID: Discord application ID" << std::endl;
}

int main(int argc, char* argv[]) {
  // Parse application ID from command line or environment
  auto application_id = ParseApplicationId(argc, argv);

  // Check if application ID is provided
  if (!application_id) {
    std::cerr << "Error: Discord Application ID is required." << std::endl;
    PrintUsage(argv[0]);
    return EXIT_FAILURE;
  }

  // Create Discord client
  auto client = std::make_shared<discordpp::Client>();

  // Create and run application
  discord_social_tui::App app(*application_id, client);
  return app.Run();
}