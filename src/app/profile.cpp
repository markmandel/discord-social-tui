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

const discordpp::UserHandle& Profile::GetUserHandle() const {
  if (!has_user_handle_ || !user_handle_.has_value()) {
    throw std::runtime_error("No user handle available");
  }
  return user_handle_.value();
}

void Profile::SetUserHandle(discordpp::UserHandle user_handle) {
  user_handle_ = std::move(user_handle);
  has_user_handle_ = true;
  spdlog::debug("Profile updated to user: {}", user_handle_->Username());
}

ftxui::Component Profile::Render() const {
  // Create a container with profile sections
  return ftxui::Renderer([this] {
    if (!has_user_handle_ || !user_handle_.has_value()) {
      return RenderEmptyProfile();
    }

    return ftxui::vbox({
        RenderUserInfo(),
        ftxui::separator(),
        RenderStatusInfo(),
        ftxui::separator(),
        RenderRelationshipInfo(),
    });
  });
}

ftxui::Element Profile::RenderEmptyProfile() {
  return ftxui::vbox({
      ftxui::text("No Profile Selected") | ftxui::bold | ftxui::center,
      ftxui::text(""),
      ftxui::paragraph("Select a user to view their profile.") | ftxui::center,
  });
}

ftxui::Element Profile::RenderUserInfo() const {
  if (!has_user_handle_ || !user_handle_.has_value()) {
    return RenderEmptyProfile();
  }

  // Get user information
  const auto& handle = user_handle_.value();
  const auto username = handle.Username();
  const auto display_name = handle.DisplayName();
  const auto user_id = std::to_string(handle.Id());
  const bool is_provisional = handle.IsProvisional();

  // Build elements for the user info section
  std::vector<ftxui::Element> elements = {
      ftxui::text("User Profile") | ftxui::bold | ftxui::center,
      ftxui::text(""),
      ftxui::hbox({
          ftxui::text("Username: ") | ftxui::bold,
          ftxui::text(username),
      })};

  // Only add display name if it's different from username
  if (!display_name.empty() && display_name != username) {
    elements.push_back(ftxui::hbox({
        ftxui::text("Display Name: ") | ftxui::bold,
        ftxui::text(display_name),
    }));
  }

  // Add user ID
  elements.push_back(ftxui::hbox({
      ftxui::text("User ID: ") | ftxui::bold,
      ftxui::text(user_id),
  }));

  // Add provisional status with appropriate styling
  elements.push_back(ftxui::hbox({
      ftxui::text("Provisional: ") | ftxui::bold,
      is_provisional ? ftxui::text("Yes") | ftxui::color(ftxui::Color::Yellow)
                     : ftxui::text("No") | ftxui::color(ftxui::Color::Green),
  }));

  // Create a vbox with all elements
  return ftxui::vbox(elements);
}

ftxui::Element Profile::RenderStatusInfo() const {
  if (!has_user_handle_ || !user_handle_.has_value()) {
    return ftxui::text("");
  }

  const auto& handle = user_handle_.value();
  std::string status_text;
  ftxui::Color status_color;

  // Determine status text and color
  switch (handle.Status()) {
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

ftxui::Element Profile::RenderRelationshipInfo() const {
  if (!has_user_handle_ || !user_handle_.has_value()) {
    return ftxui::text("");
  }

  const auto& handle = user_handle_.value();

  // Get relationship handle
  auto relationship = handle.Relationship();

  // Get relationship types
  const auto discord_relation = relationship.DiscordRelationshipType();
  const auto game_relation = relationship.GameRelationshipType();

  // Get relationship strings using EnumToString
  std::string discord_relation_text = discordpp::EnumToString(discord_relation);
  std::string game_relation_text = discordpp::EnumToString(game_relation);

  // Determine colors based on relationship types
  ftxui::Color discord_relation_color;
  switch (discord_relation) {
    case discordpp::RelationshipType::Friend:
      discord_relation_color = ftxui::Color::Green;
      break;
    case discordpp::RelationshipType::Blocked:
      discord_relation_color = ftxui::Color::Red;
      break;
    case discordpp::RelationshipType::PendingIncoming:
    case discordpp::RelationshipType::PendingOutgoing:
      discord_relation_color = ftxui::Color::Yellow;
      break;
    case discordpp::RelationshipType::None:
    default:
      discord_relation_color = ftxui::Color::GrayDark;
      break;
  }

  // Game relationship color
  ftxui::Color game_relation_color;
  switch (game_relation) {
    case discordpp::RelationshipType::Friend:
      game_relation_color = ftxui::Color::Green;
      break;
    case discordpp::RelationshipType::None:
    default:
      game_relation_color = ftxui::Color::GrayDark;
      break;
  }

  // Build relationship section
  return ftxui::vbox({
      ftxui::text("Relationship Information") | ftxui::bold | ftxui::center,
      ftxui::text(""),
      ftxui::hbox({
          ftxui::text("Discord Relationship: ") | ftxui::bold,
          ftxui::text(discord_relation_text) |
              ftxui::color(discord_relation_color),
      }),
      ftxui::hbox({
          ftxui::text("Game Relationship: ") | ftxui::bold,
          ftxui::text(game_relation_text) | ftxui::color(game_relation_color),
      }),
  });
}

}  // namespace discord_social_tui