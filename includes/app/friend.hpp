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

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "discordpp.h"
#include "ftxui/component/component.hpp"
#include "ftxui/util/ref.hpp"

namespace discord_social_tui {

// Forward declarations
class Messages;
class Voice;

// A Friend class to represent a Discord friend
class Friend {
 public:
  explicit Friend(discordpp::UserHandle user_handle,
                  std::shared_ptr<Messages> messages,
                  std::shared_ptr<Voice> voice,
                  discordpp::RelationshipGroupType group_type);

  [[nodiscard]] uint64_t GetId() const;
  [[nodiscard]] std::string GetUsername() const;
  [[nodiscard]] std::string GetDisplayName() const;
  [[nodiscard]] discordpp::StatusType GetStatus() const;
  [[nodiscard]] discordpp::RelationshipGroupType GetGroupType() const;

  // Get a display string with emoji for the friend's status
  [[nodiscard]] std::string GetFormattedDisplayName() const;

  // Implicit conversion to ftxui::ConstStringRef
  operator ftxui::ConstStringRef() const { return GetFormattedDisplayName(); }

  // Access to the underlying UserHandle
  [[nodiscard]] const discordpp::UserHandle& GetUserHandle() const {
    return user_handle_;
  }

 private:
  discordpp::UserHandle user_handle_;
  std::shared_ptr<Messages> messages_;
  std::shared_ptr<Voice> voice_;
  std::vector<discordpp::MessageHandle> message_handlers_;
  discordpp::RelationshipGroupType group_type_;
};

// A Friends class for managing and rendering a list of Discord friends
class Friends final {
 public:
  Friends(std::shared_ptr<discordpp::Client> client,
          std::shared_ptr<Messages> messages, std::shared_ptr<Voice> voice);

  // Get a friend by index
  [[nodiscard]] std::optional<std::shared_ptr<Friend>> GetFriendAt(
      size_t index) const;

  // Get a friend by ID
  [[nodiscard]] std::optional<std::shared_ptr<Friend>> GetFriendById(
      uint64_t user_id);

  // Get the number of friends
  [[nodiscard]] size_t size() const { return friends_.size(); }

  // Set the selected index to the friend with the given user ID
  void SetSelectedIndexByFriendId(uint64_t user_id);

  // Get the currently selected friend
  [[nodiscard]] std::optional<std::shared_ptr<Friend>> GetSelectedFriend()
      const {
    return GetFriendAt(selected_index_);
  }

  // Render the friends list as a menu component
  [[nodiscard]] ftxui::Component Render();

  // Refresh the menu component when friends list changes
  void Refresh();

  // Add a callback for when the selection changes
  void AddSelectionChangeHandler(std::function<void()> handler);
  // Setup initial friends list, and setup callbacks.
  void Run();

 private:
  std::vector<std::optional<std::shared_ptr<Friend>>> friends_;
  int selected_index_ = 0;       // Default to first item
  int last_selected_index_ = 0;  // Track last selection for change detection
  ftxui::Component
      menu_entries_;  // The Container::Vertical with MenuEntry items
  ftxui::Component
      menu_component_;  // The wrapped component with OnEvent handler
  std::vector<std::function<void()>> selection_change_handlers_;
  std::shared_ptr<discordpp::Client> client_;
  std::shared_ptr<Messages> messages_;
  std::shared_ptr<Voice> voice_;

  // Notify all selection change handlers
  void NotifySelectionChanged() const;
};

}  // namespace discord_social_tui