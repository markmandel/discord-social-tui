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

#include "discordpp.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

namespace discord_social_tui {

// A Profile class to display a Discord user's profile
class Profile {
 public:
  // Default constructor
  Profile() : has_user_handle_(false) {}

  // Create and return a vertical container with profile information
  [[nodiscard]] ftxui::Component Render() const;

  // Access to the underlying UserHandle
  [[nodiscard]] bool HasUserHandle() const { return has_user_handle_; }

  // Get the user handle, only call if HasUserHandle() is true
  [[nodiscard]] const discordpp::UserHandle& GetUserHandle() const;

  // Update the user handle
  void SetUserHandle(discordpp::UserHandle user_handle);

 private:
  bool has_user_handle_ = false;
  std::optional<discordpp::UserHandle> user_handle_;

  // Helper methods to create profile sections
  [[nodiscard]] ftxui::Element RenderUserInfo() const;
  [[nodiscard]] ftxui::Element RenderStatusInfo() const;
  [[nodiscard]] ftxui::Element RenderRelationshipInfo() const;
  [[nodiscard]] ftxui::Element RenderEmptyProfile() const;
};

}  // namespace discord_social_tui