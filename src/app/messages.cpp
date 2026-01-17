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

#include "app/messages.hpp"

#include <spdlog/spdlog.h>
#include <sys/stat.h>

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

namespace discord_social_tui {

Messages::Messages(const std::shared_ptr<discordpp::Client>& client)
    : client_(client) {
  // Initialize UI components
  auto option = ftxui::InputOption();
  option.multiline = false;
  option.transform = [](ftxui::InputState state) {
    if (state.focused) {
      state.element |= ftxui::bgcolor(ftxui::Color::White);
    } else if (state.hovered) {
      state.element |= ftxui::bgcolor(ftxui::Color::GrayLight);
    } else {
      state.element |= ftxui::bgcolor(ftxui::Color::GrayDark);
    }

    if (state.is_placeholder) {
      state.element |= ftxui::dim;
    } else {
      state.element |= ftxui::color(ftxui::Color::Black);
    }

    return state.element;
  };
  /// If you write anything in the input textfield, the unread thing goes away.
  option.on_change = [this]() { ResetSelectedUnreadMessages(); };
  option.on_enter = [this]() { SendMessage(); };

  input_component_ = ftxui::Input(&input_text_, "Type a message...", option);
  send_button_ = ftxui::Button("Send", [this] { SendMessage(); });
}

void Messages::SetFriends(const std::shared_ptr<Friends>& friends) {
  friends_ = friends;
}

void Messages::Run() {
  client_->SetMessageCreatedCallback(
      [this](const uint64_t message_id) { AddUserMessage(message_id); });
}

ftxui::Component Messages::Render() {
  // Create header area (friend name)
  const auto header_display = ftxui::Renderer([this] {
    if (const auto selected_friend = friends_->GetSelectedFriend();
        selected_friend.has_value()) {
      const auto& friend_ = selected_friend.value();
      return ftxui::vbox(
          {ftxui::text("Messages with " + friend_->GetDisplayName()) |
               ftxui::bold,
           ftxui::separator()});
    }
    return ftxui::vbox(
        {ftxui::text("Select a friend to view messages") | ftxui::dim,
         ftxui::separator()});
  });

  // Create scrollable messages area (only the message list scrolls)
  const auto messages_display = ftxui::Renderer([this] {
    ftxui::Elements message_elements;

    // Get currently selected friend
    if (const auto selected_friend = friends_->GetSelectedFriend()) {
      // Get and display all messages from this friend
      if (const auto& messages =
              this->GetMessages(selected_friend.value()->GetId());
          messages.empty()) {
        message_elements.push_back(ftxui::text("No messages yet...") |
                                   ftxui::dim);
      } else {
        for (const auto& message : messages) {
          // Display author and message content
          auto author_name =
              message.Author()
                  .and_then([](const discordpp::UserHandle& author)
                                -> std::optional<std::string> {
                    return author.DisplayName();
                  })
                  .value_or("<unknown>");

          message_elements.push_back(
              ftxui::hbox({ftxui::text(author_name + ": ") |
                               ftxui::color(ftxui::Color::Cyan),
                           ftxui::text(message.Content())}));
        }
      }
    }

    return ftxui::vbox(message_elements) | ftxui::vscroll_indicator |
           ftxui::yframe;
  });

  // Create input area with text field and send button (fixed at bottom)
  auto input_area = ftxui::Container::Horizontal(
      {input_component_ | ftxui::flex, send_button_});

  // Wrap input area with separator
  auto input_with_separator = ftxui::Renderer(input_area, [input_area] {
    return ftxui::vbox({ftxui::separator(), input_area->Render()});
  });

  // Combine header, scrollable messages, and fixed input area
  messages_container_ = ftxui::Container::Vertical({
      header_display,
      messages_display |
          ftxui::flex,      // Messages take up remaining space and scroll
      input_with_separator  // Input area stays at bottom
  });

  return messages_container_;
}

void Messages::SendMessage() {
  SPDLOG_INFO("Sending message: {}", input_text_);
  friends_->GetSelectedFriend().and_then(
      [this](const std::shared_ptr<Friend>& friend_)
          -> std::optional<std::monostate> {
        if (input_text_.empty()) {
          SPDLOG_DEBUG("Cannot send empty message");
          return std::nullopt;
        }

        client_->SendUserMessage(
            friend_->GetId(), input_text_,
            [this, friend_](const discordpp::ClientResult& result,
                            unsigned long message_id) {
              if (!result.Successful()) {
                SPDLOG_ERROR("Failed to send message: {}", result.Error());
                return;
              }
              input_text_.clear();
              SPDLOG_INFO("Message sent: {}", message_id);
            });
        return std::monostate{};
      });
}

void Messages::AddUserMessage(const uint64_t message_id) {
  client_->GetMessageHandle(message_id)
      .and_then([this](const discordpp::MessageHandle& message)
                    -> std::optional<std::monostate> {
        SPDLOG_INFO("New message received: {} - {}", message.AuthorId(),
                    message.Content());
        const auto current_user = client_->GetCurrentUserV2();

        if (!current_user) {
          SPDLOG_ERROR("Current user not available");
          return std::nullopt;
        }

        uint64_t user_id = 0;
        // store my own messages against the recipient
        if (message.AuthorId() == current_user->Id()) {
          user_id = message.RecipientId();
        } else {
          user_id = message.AuthorId();

          // if nothing selected, or not on the same user, then set it to not
          // being read.
          friends_->GetSelectedFriend()
              .and_then([this, user_id](const std::shared_ptr<Friend>& friend_)
                            -> std::optional<std::monostate> {
                // TODO: also check if the messages component is being
                // displayed, if not it doesn't matter if the selected user is
                // the same.
                if (friend_->GetId() != user_id) {
                  unread_messages_[user_id] = true;
                }
                return std::monostate{};
              })
              .or_else([this, user_id] -> std::optional<std::monostate> {
                unread_messages_[user_id] = true;
                return std::monostate{};
              });
        }

        if (!user_messages_.contains(user_id)) {
          user_messages_[user_id] = std::vector<discordpp::MessageHandle>();
        }
        user_messages_[user_id].push_back(message);
        OnUnreadChange();

        return std::monostate{};
      });
}

void Messages::ResetSelectedUnreadMessages() {
  friends_->GetSelectedFriend().and_then(
      [this](const std::shared_ptr<Friend>& friend_)
          -> std::optional<std::monostate> {
        unread_messages_[friend_->GetId()] = false;
        OnUnreadChange();
        return std::monostate{};
      });
}

std::vector<discordpp::MessageHandle> Messages::GetMessages(
    const uint64_t user_id) {
  if (!user_messages_.contains(user_id)) {
    // Initialise empty vector to indicate we're fetching
    user_messages_[user_id] = std::vector<discordpp::MessageHandle>();

    // Fetch message history from Discord API
    constexpr int32_t message_limit = 50;
    client_->GetUserMessagesWithLimit(
        user_id, message_limit,
        [this, user_id](const discordpp::ClientResult& result,
                        const std::vector<discordpp::MessageHandle>& messages) {
          if (!result.Successful()) {
            SPDLOG_ERROR("Failed to fetch message history for user {}: {}",
                         user_id, result.Error());
            return;
          }

          SPDLOG_INFO("Fetched {} historical messages for user {}",
                      messages.size(), user_id);
          user_messages_[user_id] = messages;
        });
  }

  return user_messages_[user_id];
}

bool Messages::HasUnreadMessages(const uint64_t user_id) const {
  if (!unread_messages_.contains(user_id)) {
    return false;
  }
  return unread_messages_.at(user_id);
}

void Messages::AddUnreadChangeHandler(std::function<void()> handler) {
  unread_change_handlers_.push_back(std::move(handler));
}

void Messages::OnUnreadChange() const {
  for (const auto& handler : unread_change_handlers_) {
    handler();
  }
}

}  // namespace discord_social_tui