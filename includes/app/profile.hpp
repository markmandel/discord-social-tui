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

#pragma once

#include <memory>
#include <optional>
#include <string>

#include "app/friend.hpp"
#include "discordpp.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

namespace discord_social_tui {

class Friends;

// A Profile class to display a Discord user's profile
class Profile {
 public:
  // Constructor with Friends
  explicit Profile(std::shared_ptr<Friends> friends);

  // Create and return a vertical container with profile information
  [[nodiscard]] ftxui::Component Render() const;

 private:
  std::shared_ptr<Friends> friends_;

  // Helper methods to create profile sections
  [[nodiscard]] ftxui::Element RenderUserInfo(
      discordpp::UserHandle user_handle) const;
  [[nodiscard]] ftxui::Element RenderStatusInfo(
      discordpp::UserHandle user_handle) const;
  [[nodiscard]] ftxui::Element RenderRelationshipInfo(
      discordpp::UserHandle user_handle) const;
  [[nodiscard]] static ftxui::Element RenderEmptyProfile();
};

}  // namespace discord_social_tui