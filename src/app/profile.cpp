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

#include "app/profile.hpp"

#include <spdlog/spdlog.h>

#include <string>
#include <utility>

namespace discord_social_tui {

Profile::Profile(discordpp::UserHandle user_handle)
    : user_handle_(std::move(user_handle)) {}

void Profile::SetUserHandle(discordpp::UserHandle user_handle) {
  user_handle_ = std::move(user_handle);
  spdlog::debug("Profile updated to user: {}", user_handle_.Username());
}

ftxui::Component Profile::Render() const {
  // Create a container with profile sections
  return ftxui::Renderer([this] {
    return ftxui::vbox({
        RenderUserInfo(),
        ftxui::separator(),
        RenderStatusInfo(),
    });
  });
}

ftxui::Element Profile::RenderUserInfo() const {
  // Get user information
  const auto username = user_handle_.Username();
  const auto display_name = user_handle_.DisplayName();
  const auto user_id = std::to_string(user_handle_.Id());

  // Build user info section
  auto user_info = ftxui::vbox({
      ftxui::text("User Profile") | ftxui::bold | ftxui::center,
      ftxui::text(""),
      ftxui::hbox({
          ftxui::text("Username: ") | ftxui::bold,
          ftxui::text(username),
      }),
  });

  // Only add display name if it's different from username
  if (!display_name.empty() && display_name != username) {
    user_info->Add(ftxui::hbox({
        ftxui::text("Display Name: ") | ftxui::bold,
        ftxui::text(display_name),
    }));
  }

  // Add user ID
  user_info->Add(ftxui::hbox({
      ftxui::text("User ID: ") | ftxui::bold,
      ftxui::text(user_id),
  }));

  return user_info;
}

ftxui::Element Profile::RenderStatusInfo() const {
  std::string status_text;
  ftxui::Color status_color;

  // Determine status text and color
  switch (user_handle_.Status()) {
    case discordpp::StatusType::Online:
      status_text = "Online";
      status_color = ftxui::Color::Green;
      break;
    case discordpp::StatusType::Idle:
      status_text = "Idle";
      status_color = ftxui::Color::YellowLight;
      break;
    case discordpp::StatusType::Blocked:
      status_text = "Blocked";
      status_color = ftxui::Color::Red;
      break;
    case discordpp::StatusType::Offline:
    default:
      status_text = "Offline";
      status_color = ftxui::Color::GrayDark;
      break;
  }

  // Build status section
  return ftxui::vbox({
      ftxui::text("Current Status") | ftxui::bold | ftxui::center,
      ftxui::text(""),
      ftxui::hbox({
          ftxui::text("Status: ") | ftxui::bold,
          ftxui::text(status_text) | ftxui::color(status_color),
      }),
      // Add additional status information here if needed
  });
}

}  // namespace discord_social_tui