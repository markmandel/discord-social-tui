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

#include "app/presence.hpp"

#include <spdlog/spdlog.h>

namespace discord_social_tui {

void Presence::SetDefaultPresence() const {
  discordpp::Activity activity;
  activity.SetType(discordpp::ActivityTypes::Playing);
  activity.SetState("Discord on the Command Line");
  activity.SetDetails("Better TUI than me...");

  SPDLOG_INFO("Updating Discord rich presence...");
  client_->UpdateRichPresence(activity,
                              [](const discordpp::ClientResult& result) {
                                if (result.Successful()) {
                                  SPDLOG_INFO(
                                      "Rich Presence updated "
                                      "successfully");
                                } else {
                                  SPDLOG_ERROR(
                                      "Rich Presence update failed: "
                                      "{}",
                                      result.Error());
                                }
                              });
}

void Presence::SetVoiceCallPresence(const std::string& lobby_secret,
                                    const OnSuccessCallback& on_success) const {
  auto activity = discordpp::Activity();
  activity.SetType(discordpp::ActivityTypes::Playing);
  activity.SetDetails("Making a phone call...");
  activity.SetSupportedPlatforms(discordpp::ActivityGamePlatforms::Desktop);

  auto secrets = discordpp::ActivitySecrets();
  secrets.SetJoin(lobby_secret);
  activity.SetSecrets(secrets);

  auto party = discordpp::ActivityParty();
  party.SetId(lobby_secret);
  party.SetCurrentSize(1);
  party.SetMaxSize(2);
  party.SetPrivacy(discordpp::ActivityPartyPrivacy::Private);
  activity.SetParty(party);

  SPDLOG_INFO("Updating Discord rich presence for voice call...");
  client_->UpdateRichPresence(
      activity, [on_success](const discordpp::ClientResult& result) {
        if (result.Successful()) {
          SPDLOG_INFO("Voice call presence updated successfully");
          if (on_success) {
            on_success();
          }
        } else {
          SPDLOG_ERROR("Voice call presence update failed: {}", result.Error());
        }
      });
}

}  // namespace discord_social_tui
