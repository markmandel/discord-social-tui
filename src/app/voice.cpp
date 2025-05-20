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

#include "app/voice.hpp"

#include <spdlog/spdlog.h>

#include <string>

namespace discord_social_tui {
void Voice::Call(std::shared_ptr<discord_social_tui::Friend> friend_) {
  // let's build an Activity.
  const std::string lobby_secret = "call::" + friend_->GetUsername();

  spdlog::info("Invoking Voice::Call! {}", lobby_secret);

  client_->CreateOrJoinLobby(
      lobby_secret,
      [&](const discordpp::ClientResult& result, unsigned long lobbyId) {
        // TODO: send an invite to the above friend.

        if (!result.Successful()) {
          spdlog::error("Failed to create or join lobby: {}", result.Error());
          return;
        }

        // Create activity and set properties
        discordpp::Activity activity;
        activity.SetType(discordpp::ActivityTypes::Playing);
        activity.SetDetails("Making a phone call...");

        // set name, state, party size ...
        discordpp::ActivitySecrets secrets{};
        secrets.SetJoin(lobby_secret);
        activity.SetSecrets(secrets);

        spdlog::info("Join Activity Secrets: {}",
                     activity.Secrets().value().Join());

        client_->UpdateRichPresence(
            activity, [&](const discordpp::ClientResult& result) {
              if (!result.Successful()) {
                spdlog::error("Failed to update rich presence: {}",
                              result.Error());
                return;
              }

              client_->SendActivityInvite(
                  friend_->GetId(), "Voice Call",
                  [](const discordpp::ClientResult& result) {
                    if (!result.Successful()) {
                      spdlog::error("Failed to send Voice Call invite: {}",
                                    result.Error());
                      return;
                    }
                    spdlog::info("☎️ Voice Call successfully called");
                  });
            });
      });
}
}  // namespace discord_social_tui