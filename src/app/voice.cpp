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
  auto current_user = client_->GetCurrentUser();
  const std::string lobby_secret = VOICE_CALL_PREFIX + current_user.Username() +
                                   ":" + friend_->GetUsername();

  SPDLOG_INFO("Invoking Voice::Call! {}", lobby_secret);

  client_->CreateOrJoinLobby(
      lobby_secret,
      [this, friend_, lobby_secret](const discordpp::ClientResult& result,
                                    unsigned long lobbyId) {
        if (!result.Successful()) {
          SPDLOG_ERROR("Failed to create or join lobby: {}", result.Error());
          return;
        }

        // Create activity and set properties
        auto activity = discordpp::Activity();
        activity.SetType(discordpp::ActivityTypes::Playing);
        activity.SetDetails("Making a phone call...");
        activity.SetSupportedPlatforms(
            discordpp::ActivityGamePlatforms::Desktop);

        // set name, state, party size ...
        auto secrets = discordpp::ActivitySecrets();
        secrets.SetJoin(lobby_secret);
        activity.SetSecrets(secrets);

        auto party = discordpp::ActivityParty();
        party.SetId(lobby_secret);
        party.SetCurrentSize(1);
        party.SetMaxSize(2);
        party.SetPrivacy(discordpp::ActivityPartyPrivacy::Private);
        activity.SetParty(party);

        client_->UpdateRichPresence(
            activity, [&](const discordpp::ClientResult& result) {
              if (!result.Successful()) {
                SPDLOG_ERROR("Failed to update rich presence: {}",
                             result.Error());
                return;
              }

              client_->SendActivityInvite(
                  friend_->GetId(), "Voice Call",
                  [](const discordpp::ClientResult& result) {
                    if (!result.Successful()) {
                      SPDLOG_ERROR("Failed to send Voice Call invite: {}",
                                   result.Error());
                      return;
                    }
                    SPDLOG_INFO("☎️ Voice Call successfully invited");
                  });
            });
      });
}

void Voice::Run() const {
  SPDLOG_INFO("Starting Voice Service...");

  client_->SetActivityInviteCreatedCallback(
      [&](const discordpp::ActivityInvite& invite) {
        SPDLOG_INFO("Received activity invite: {}", invite.PartyId());

        // this is a voice call, so auto accept and start voice call
        if (invite.PartyId().starts_with(VOICE_CALL_PREFIX)) {
          SPDLOG_INFO("Invite is a voice invite, so accepting it...");
          client_->AcceptActivityInvite(
              invite, [&](const discordpp::ClientResult& result,
                          std::string lobby_secret) {
                if (!result.Successful()) {
                  SPDLOG_ERROR("Could not accept activity invite: {}",
                               result.Error());
                  return;
                }

                SPDLOG_INFO("Joining lobby with secret: {}", lobby_secret);
                client_->CreateOrJoinLobby(
                    lobby_secret,
                    [this, lobby_secret](const discordpp::ClientResult& result,
                                         unsigned long lobbyId) {
                      if (!result.Successful()) {
                        SPDLOG_ERROR("Failed to join lobby: {}",
                                     result.Error());
                      }

                      // TODO: start voice call
                    });
              });
        }
      });
}
}  // namespace discord_social_tui