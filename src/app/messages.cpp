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

Messages::Messages(const std::shared_ptr<discordpp::Client>& client,
                   const std::shared_ptr<Friends>& friends)
    : client_(client), friends_(friends) {
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

void Messages::Run() const {
  client_->SetMessageCreatedCallback([this](const uint64_t message_id) {
    client_->GetMessageHandle(message_id)
        .and_then([this](const discordpp::MessageHandle& message)
                      -> std::optional<std::monostate> {
          SPDLOG_INFO("New message received: {}", message.Content());

          return friends_->GetFriendById(message.AuthorId())
              .and_then([message](const std::shared_ptr<Friend>& friend_)
                            -> std::optional<std::monostate> {
                friend_->AddMessage(message);
                return std::monostate{};
              });
        });
  });
}

void Messages::ResetSelectedUnreadMessages() const {
  friends_->GetSelectedFriend().and_then(
      [](const std::shared_ptr<Friend>& friend_)
          -> std::optional<std::monostate> {
        friend_->ResetUnreadMessages();
        return std::monostate{};
      });
}

ftxui::Component Messages::Render() {
  // Create header area (friend name)
  const auto header_display = ftxui::Renderer([this] {
    const auto selected_friend = friends_->GetSelectedFriend();
    if (selected_friend.has_value()) {
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
      const auto& friend_ = selected_friend.value();

      // Get and display all messages from this friend
      const auto& messages = friend_->GetMessages();
      if (messages.empty()) {
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
                SPDLOG_INFO("Failed to send message: {}", result.Error());
                return;
              }
              input_text_.clear();

              SPDLOG_INFO("Message sent: {}", message_id);
              client_->GetMessageHandle(message_id)
                  .and_then([friend_](const discordpp::MessageHandle& message)
                                -> std::optional<std::monostate> {
                    friend_->AddMessage(message);
                    return std::monostate{};
                  });
            });
        return std::monostate{};
      });
}

}  // namespace discord_social_tui