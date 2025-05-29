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

namespace discord_social_tui {

Messages::Messages(const std::shared_ptr<discordpp::Client>& client,
                   const std::shared_ptr<Friends>& friends)
    : client_(client), friends_(friends) {}

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

}  // namespace discord_social_tui