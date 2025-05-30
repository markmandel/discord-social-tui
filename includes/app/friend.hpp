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
#include "ftxui/util/ref.hpp"

namespace discord_social_tui {

// A Friend class to represent a Discord friend
class Friend {
 public:
  explicit Friend(discordpp::UserHandle user_handle);

  [[nodiscard]] uint64_t GetId() const;
  [[nodiscard]] std::string GetUsername() const;
  [[nodiscard]] std::string GetDisplayName() const;
  [[nodiscard]] discordpp::StatusType GetStatus() const;

  // Get a display string with emoji for the friend's status
  [[nodiscard]] std::string GetFormattedDisplayName() const;

  // Access to the underlying UserHandle
  [[nodiscard]] const discordpp::UserHandle& GetUserHandle() const {
    return user_handle_;
  }

  // Operators for comparing friends (implements Comparable and Sortable
  // concepts)
  bool operator<(const Friend& other) const;
  bool operator>(const Friend& other) const;
  bool operator<=(const Friend& other) const;
  bool operator>=(const Friend& other) const;

  // Operators for equality (implements Regular concept)
  bool operator==(const Friend& other) const;
  bool operator!=(const Friend& other) const;

  // Get status priority (lower number = higher priority)
  static int GetStatusPriority(discordpp::StatusType status);

  // Voice call management
  [[nodiscard]] const std::optional<discordpp::Call>& GetVoiceCall() const;
  void SetVoiceCall(const std::optional<discordpp::Call>& call);
  void ClearVoiceCall();

  // Message handler management
  void AddMessage(const discordpp::MessageHandle& message);
  [[nodiscard]] const std::vector<discordpp::MessageHandle>& GetMessages()
      const;

 private:
  discordpp::UserHandle user_handle_;
  std::optional<discordpp::Call> voice_call_;
  std::vector<discordpp::MessageHandle> message_handlers_;
};

// A Friends adapter class for FTXUI menus
class Friends : public ftxui::ConstStringListRef::Adapter {
 public:
  Friends() = default;

  // Add a friend to the list and maintain sorting
  void AddFriend(std::shared_ptr<Friend> friend_);

  // Remove a friend from the list by ID
  void RemoveFriend(uint64_t user_id);

  // Get a friend by index
  [[nodiscard]] std::optional<std::shared_ptr<Friend>> GetFriendAt(
      size_t index) const;

  // Get a friend by ID
  [[nodiscard]] std::optional<std::shared_ptr<Friend>> GetFriendById(
      uint64_t user_id);

  // Sort the friends list by status (Online, Idle, Offline, Blocked) then
  // alphabetically, using the Friend class's operators
  void SortFriends();

  // Implement the ConstStringListRef::Adapter interface
  [[nodiscard]] size_t size() const override { return friends_.size(); }
  [[nodiscard]] std::string operator[](size_t index) const override;

  // Get/set the currently selected friend index
  [[nodiscard]] int* GetSelectedIndex() { return &selected_index_; }
  void SetSelectedIndex(const int index) {
    selected_index_ = std::min(index, static_cast<int>(friends_.size() - 1));
  }
  // Set the selected index to the friend with the given user ID
  void SetSelectedIndexByFriendId(uint64_t user_id);
  [[nodiscard]] std::optional<std::shared_ptr<Friend>> GetSelectedFriend()
      const {
    return GetFriendAt(selected_index_);
  }

 private:
  std::vector<std::shared_ptr<Friend>> friends_;
  int selected_index_ = 0;  // Default to first item
};

}  // namespace discord_social_tui