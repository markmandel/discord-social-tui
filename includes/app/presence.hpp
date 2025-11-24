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
#include <string>

#include "discordpp.h"

namespace discord_social_tui {

/// Manages Discord Rich Presence updates for the application.
/// Encapsulates all calls to client->UpdateRichPresence() and provides
/// convenient methods for different presence scenarios.
class Presence {
 public:
  /// Callback type for successful presence updates
  using OnSuccessCallback = std::function<void()>;

  explicit Presence(const std::shared_ptr<discordpp::Client>& client)
      : client_(client) {}

  /// Set the default application presence shown when the app is ready.
  /// Shows "Discord on the Command Line" with custom details.
  /// Handles success/failure logging internally.
  void SetDefaultPresence() const;

  /// Set presence for an active voice call.
  /// Creates a joinable activity with lobby information and party details.
  /// Handles success/failure logging internally.
  /// @param lobby_secret The lobby secret for the voice call
  /// @param on_success Callback to invoke when presence is set successfully
  void SetVoiceCallPresence(const std::string& lobby_secret,
                            const OnSuccessCallback& on_success) const;

 private:
  std::shared_ptr<discordpp::Client> client_;
};

}  // namespace discord_social_tui
