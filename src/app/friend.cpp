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

int Friend::GetStatusPriority(const discordpp::StatusType status) {
  // Order of priority: Online, Idle, Offline, Blocked
  switch (status) {
    case discordpp::StatusType::Online:
      return 0;
    case discordpp::StatusType::Idle:
      return 1;
    case discordpp::StatusType::Offline:
      return 2;
    case discordpp::StatusType::Blocked:
      return 3;
    default:
      return 4;  // Any unknown status comes last
  }
}

bool Friend::operator<(const Friend& other) const {
  spdlog::debug("Friend::operator<");
  // First compare by status priority
  const int status_a = GetStatusPriority(GetStatus());
  const int status_b = GetStatusPriority(other.GetStatus());

  if (status_a != status_b) {
    return status_a < status_b;  // Lower priority number comes first
  }

  // If status is the same, compare alphabetically by display name
  if (GetDisplayName() != other.GetDisplayName()) {
    return GetDisplayName() < other.GetDisplayName();
  }

  // stable sorting
  return GetId() < other.GetId();
}

bool Friend::operator>(const Friend& other) const {
  // Reuse the < operator for consistency
  return other < *this;
}

bool Friend::operator<=(const Friend& other) const {
  // a <= b is equivalent to !(a > b)
  return !(*this > other);
}

bool Friend::operator>=(const Friend& other) const {
  // a >= b is equivalent to !(a < b)
  return !(*this < other);
}

bool Friend::operator==(const Friend& other) const {
  // Friends are equal if they have the same ID
  return GetId() == other.GetId();
}

bool Friend::operator!=(const Friend& other) const {
  // a != b is equivalent to !(a == b)
  return !(*this == other);
}

static_assert(std::equality_comparable<Friend>);
static_assert(std::totally_ordered<Friend>);

void Friends::AddFriend(std::unique_ptr<Friend> friend_) {
  spdlog::debug("Adding friend: {}", friend_->GetUsername());

  // Store username for logging as we'll be moving the friend
  std::string username = friend_->GetUsername();

  // Find the position to insert using binary search
  // We need to dereference the unique_ptrs to compare Friend objects
  auto pos = std::ranges::lower_bound(
      friends_, friend_, 
      [](const auto& friend_a, const auto& friend_b) {
        // Compare the Friend objects using the operator< we defined
        return *friend_a < *friend_b;
      });

  // Insert the friend at the correct position
  friends_.insert(pos, std::move(friend_));
  spdlog::debug("Friend {} inserted at position {}", username,
                std::distance(friends_.begin(), pos));
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

void Friends::SortFriends() {
  ::spdlog::debug("Sorting Friends");
  // Use projection to dereference the unique_ptr and compare the Friend objects
  std::ranges::sort(friends_, {}, [](const auto& ptr) -> const Friend& { return *ptr; });
}

std::string Friends::operator[](const size_t index) const {
  if (index < friends_.size()) {
    return friends_[index]->GetFormattedDisplayName();
  }
  return "";
}

}  // namespace discord_social_tui