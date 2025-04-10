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

#include "app/friend.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>

namespace discord_social_tui {

Friend::Friend(discordpp::UserHandle user_handle)
    : user_handle_(std::move(user_handle)) {}

uint64_t Friend::GetId() const { return user_handle_.Id(); }

std::string Friend::GetUsername() const { return user_handle_.Username(); }

std::string Friend::GetDisplayName() const {
  auto display_name = user_handle_.DisplayName();
  if (!display_name.empty()) {
    return display_name;
  }
  // Fall back to username if display name is not available
  return GetUsername();
}

discordpp::StatusType Friend::GetStatus() const {
  return user_handle_.Status();
}

std::string Friend::GetFormattedDisplayName() const {
  std::string status_emoji;

  switch (GetStatus()) {
    case discordpp::StatusType::Online:
      status_emoji = "🟢";  // Green circle for online
      break;
    case discordpp::StatusType::Idle:
      status_emoji = "🟡";  // Yellow circle for idle
      break;
    case discordpp::StatusType::Blocked:
      status_emoji = "⛔";  // No entry sign for blocked
      break;
    case discordpp::StatusType::Offline:
    default:
      status_emoji = "⚫";  // Black circle for offline
      break;
  }

  return status_emoji + " " + GetDisplayName();
}

void Friends::AddFriend(std::unique_ptr<Friend> friend_) {
  spdlog::debug("Adding friend: {}", friend_->GetUsername());
  friends_.push_back(std::move(friend_));
}

void Friends::RemoveFriend(uint64_t user_id) {
  const auto iterator = std::ranges::find_if(
      friends_,
      [user_id](const auto& friend_) { return friend_->GetId() == user_id; });

  if (iterator != friends_.end()) {
    spdlog::debug("Removing friend: {}", (*iterator)->GetUsername());
    friends_.erase(iterator);
  } else {
    spdlog::warn("Friend not found with ID: {}", user_id);
  }
}

Friend* Friends::GetFriendAt(const size_t index) const {
  if (index < friends_.size()) {
    return friends_[index].get();
  }
  return nullptr;
}

Friend* Friends::GetFriendById(uint64_t user_id) {
  const auto iterator = std::ranges::find_if(
      friends_,
      [user_id](const auto& friend_) { return friend_->GetId() == user_id; });

  return (iterator != friends_.end()) ? iterator->get() : nullptr;
}

std::string Friends::operator[](size_t index) const {
  if (index < friends_.size()) {
    return friends_[index]->GetFormattedDisplayName();
  }
  return "";
}

}  // namespace discord_social_tui