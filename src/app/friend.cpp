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

#include "app/messages.hpp"
#include "app/voice.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"

namespace discord_social_tui {

Friend::Friend(discordpp::UserHandle user_handle,
               std::shared_ptr<Messages> messages, std::shared_ptr<Voice> voice,
               const discordpp::RelationshipGroupType group_type)
    : user_handle_(std::move(user_handle)),
      messages_(std::move(messages)),
      voice_(std::move(voice)),
      group_type_(group_type) {}

uint64_t Friend::GetId() const { return user_handle_.Id(); }

std::string Friend::GetUsername() const { return user_handle_.Username(); }

std::string Friend::GetDisplayName() const {
  if (auto display_name = user_handle_.DisplayName(); !display_name.empty()) {
    return display_name;
  }
  // Fall back to username if display name is not available
  return GetUsername();
}

discordpp::StatusType Friend::GetStatus() const {
  return user_handle_.Status();
}

discordpp::RelationshipGroupType Friend::GetGroupType() const {
  return group_type_;
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
    case discordpp::StatusType::Dnd:
      status_emoji = "🔴";  // Red circle for do not disturb
      break;
    case discordpp::StatusType::Invisible:
      status_emoji = "⚪";  // White circle for invisible
      break;
    case discordpp::StatusType::Offline:
    default:
      status_emoji = "⚫";  // Black circle for offline
      break;
  }

  if (voice_->GetCall(GetId()).has_value()) {
    status_emoji += "🔉";
  }

  if (messages_->HasUnreadMessages(GetId())) {
    status_emoji += "📨";
  }

  return status_emoji + " " + GetDisplayName();
}

Friends::Friends(std::shared_ptr<discordpp::Client> client,
                 std::shared_ptr<Messages> messages,
                 std::shared_ptr<Voice> voice)
    : menu_entries_(ftxui::Container::Vertical(std::vector<ftxui::Component>{},
                                               &selected_index_)),
      client_(std::move(client)),
      messages_(std::move(messages)),
      voice_(std::move(voice)) {
  // Re-wrap with OnEvent handler and scrolling
  this->menu_component_ =
      menu_entries_ | ftxui::CatchEvent([this](const ftxui::Event&) -> bool {
        if (selected_index_ != last_selected_index_) {
          last_selected_index_ = selected_index_;
          NotifySelectionChanged();
        }
        return false;  // Don't consume the event
      }) |
      ftxui::vscroll_indicator | ftxui::yframe;
}

std::optional<std::shared_ptr<Friend>> Friends::GetFriendAt(
    const size_t index) const {
  if (index < friends_.size()) {
    // Return the optional value (which may be nullopt for headers)
    return friends_[index];
  }
  return std::nullopt;
}

std::optional<std::shared_ptr<Friend>> Friends::GetFriendById(
    uint64_t user_id) {
  const auto iterator =
      std::ranges::find_if(friends_, [user_id](const auto& friend_opt) {
        return friend_opt.has_value() && friend_opt.value()->GetId() == user_id;
      });

  return (iterator != friends_.end()) ? *iterator : std::nullopt;
}

void Friends::SetSelectedIndexByFriendId(uint64_t user_id) {
  for (size_t i = 0; i < friends_.size(); ++i) {
    if (auto friend_ = friends_[i];
        friend_ && friend_.value()->GetId() == user_id) {
      selected_index_ = static_cast<int>(i);
      return;
    }
  }
  // If friend is not found, keep the current selection
  spdlog::warn("Friend with ID {} not found, keeping current selection",
               user_id);
}

void Friends::NotifySelectionChanged() const {
  for (const auto& handler : selection_change_handlers_) {
    handler();
  }
}

void Friends::AddSelectionChangeHandler(std::function<void()> handler) {
  selection_change_handlers_.push_back(std::move(handler));
}

void Friends::Run() {
  // Set up the unified friends list update callback
  client_->SetRelationshipGroupsUpdatedCallback(
      [this](const uint64_t _user_id) { Refresh(); });
  voice_->AddChangeHandler([this] { Refresh(); });
  messages_->AddUnreadChangeHandler([this] { Refresh(); });
}

void Friends::Refresh() {
  SPDLOG_INFO("Refreshing friends list");
  if (!menu_entries_) {
    SPDLOG_WARN("Cannot refresh friends list: menu component not yet created");
    return;
  }

  const auto selected_id =
      GetSelectedFriend()
          .transform([](const std::shared_ptr<Friend>& friend_) {
            return friend_->GetId();
          })
          .value_or(-1);

  const auto online_playing = client_->GetRelationshipsByGroup(
      discordpp::RelationshipGroupType::OnlinePlayingGame);
  const auto online_elsewhere = client_->GetRelationshipsByGroup(
      discordpp::RelationshipGroupType::OnlineElsewhere);
  const auto offline = client_->GetRelationshipsByGroup(
      discordpp::RelationshipGroupType::Offline);

  // rebuild friends, so we can find them again.
  friends_.clear();
  // Remove all menu entries and rebuild!
  menu_entries_->DetachAllChildren();

  // Add "Online Playing" header
  menu_entries_->Add(
      ftxui::Renderer([] { return ftxui::text("Online Playing"); }));
  friends_.emplace_back(std::nullopt);  // Header position

  for (const auto& relationship : online_playing) {
    if (auto user = relationship.User()) {
      auto friend_ = std::make_shared<Friend>(
          user.value(), messages_, voice_,
          discordpp::RelationshipGroupType::OnlinePlayingGame);
      menu_entries_->Add(ftxui::MenuEntry(*friend_));
      friends_.emplace_back(std::move(friend_));
    }
  }

  // Add "Online Elsewhere" header
  menu_entries_->Add(
      ftxui::Renderer([] { return ftxui::text("Online Elsewhere"); }));
  friends_.emplace_back(std::nullopt);  // Header position

  for (const auto& relationship : online_elsewhere) {
    if (auto user = relationship.User()) {
      auto friend_ = std::make_shared<Friend>(
          user.value(), messages_, voice_,
          discordpp::RelationshipGroupType::OnlineElsewhere);
      menu_entries_->Add(ftxui::MenuEntry(*friend_));
      friends_.emplace_back(std::move(friend_));
    }
  }

  // Add "Offline" header
  menu_entries_->Add(ftxui::Renderer([] { return ftxui::text("Offline"); }));
  friends_.emplace_back(std::nullopt);  // Header position

  for (const auto& relationship : offline) {
    if (auto user = relationship.User()) {
      auto friend_ =
          std::make_shared<Friend>(user.value(), messages_, voice_,
                                   discordpp::RelationshipGroupType::Offline);
      menu_entries_->Add(ftxui::MenuEntry(*friend_));
      friends_.emplace_back(std::move(friend_));
    }
  }

  // keep pointing at the same person
  if (selected_id > 0) {
    SetSelectedIndexByFriendId(selected_id);
  }
}

ftxui::Component Friends::Render() {
  // Build initial menu entries
  return menu_component_;
}

}  // namespace discord_social_tui