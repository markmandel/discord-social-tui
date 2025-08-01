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
#include <string>
#include <unordered_map>
#include <vector>

#include "app/friend.hpp"
#include "discordpp.h"
#include "ftxui/component/component.hpp"

namespace discord_social_tui {

class Messages {
 public:
  explicit Messages(const std::shared_ptr<discordpp::Client>& client,
                    const std::shared_ptr<Friends>& friends);
  void Run();
  /// Resets the selected user has unread messages.
  void ResetSelectedUnreadMessages();
  // Does this user have any unread messages?
  bool HasUnreadMessages(uint64_t user_id) const;

  /// Render the messages UI component
  [[nodiscard]] ftxui::Component Render();

 private:
  std::shared_ptr<discordpp::Client> client_;
  std::shared_ptr<Friends> friends_;
  std::string input_text_;
  ftxui::Component input_component_;
  ftxui::Component send_button_;
  ftxui::Component messages_container_;
  std::unordered_map<uint64_t, std::vector<discordpp::MessageHandle>>
      user_messages_;
  // does the user have unread messages
  std::unordered_map<u_int64_t, bool> unread_messages_;

  void SendMessage();
  void AddUserMessage(uint64_t message_id);
  std::vector<discordpp::MessageHandle> GetMessages(uint64_t user_id);
};

}  // namespace discord_social_tui