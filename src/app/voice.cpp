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

void Voice::Call() {
  const auto current_user = client_->GetCurrentUserV2();
  const auto selected_friend = friends_->GetSelectedFriend();

  if (!current_user) {
    SPDLOG_ERROR("Current user not available");
    return;
  }

  if (!selected_friend) {
    SPDLOG_ERROR("No friend selected for voice call");
    return;
  }

  const auto& friend_ = selected_friend.value();
  const std::string lobby_secret = VOICE_CALL_PREFIX +
                                   current_user->Username() + ":" +
                                   friend_->GetUsername();

  SPDLOG_INFO("Invoking Voice::Call! {}", lobby_secret);

  client_->CreateOrJoinLobby(
      lobby_secret,
      [this, friend_, lobby_secret](const discordpp::ClientResult& result,
                                    unsigned long lobby_id) {
        if (!result.Successful()) {
          SPDLOG_ERROR("Failed to create or join lobby: {}", result.Error());
          return;
        }

        // Update rich presence for voice call
        presence_->SetVoiceCallPresence(lobby_secret, [this, friend_, lobby_id] {
          // Send activity invite after presence is set
          client_->SendActivityInvite(
              friend_->GetId(), "Voice Call",
              [this, friend_, lobby_id](const discordpp::ClientResult& result) {
                if (!result.Successful()) {
                  SPDLOG_ERROR("Failed to send Voice Call invite: {}",
                               result.Error());
                  return;
                }
                SPDLOG_INFO("☎️ Voice Call successfully invited");
                const discordpp::Call call = client_->StartCall(lobby_id);
                active_calls_.insert({friend_->GetId(), call});
                OnChange();
              });
        });
      });
}

// TODO: Update rich presence when the other person joins the call.

void Voice::Disconnect() {
  friends_->GetSelectedFriend().and_then(
      [this](const std::shared_ptr<Friend>& friend_)
          -> std::optional<std::monostate> {
        return GetCall(friend_->GetId())
            .and_then([this, friend_](const discordpp::Call& call)
                          -> std::optional<std::monostate> {
              client_->EndCall(call.GetChannelId(), [this, friend_]() {
                active_calls_.erase(friend_->GetId());
                // TODO: Update rich presence to turn off the call.
                OnChange();
                SPDLOG_INFO("Call ended successfully!");
              });

              return std::monostate{};
            });
      });
}

void Voice::Run() {
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
                                         unsigned long lobby_id) {
                      if (!result.Successful()) {
                        SPDLOG_ERROR("Failed to join lobby: {}",
                                     result.Error());
                      }

                      SPDLOG_INFO("Starting voice call with lobby ID: {}",
                                  lobby_id);
                      auto call = client_->StartCall(lobby_id);

                      const auto participants = call.GetParticipants();
                      if (participants.empty()) {
                        SPDLOG_WARN("No participants in the call");
                        return;
                      }
                      friends_->GetFriendById(participants[0])
                          .and_then([this, call](
                                        const std::shared_ptr<Friend>& friend_)
                                        -> std::optional<std::monostate> {
                            active_calls_.insert({friend_->GetId(), call});
                            // TODO: update rich presence when you join a call.
                            OnChange();
                            return std::monostate{};
                          });
                    });
              });
        }
      });
}

// TODO: not quite sure how to do this, but if the other person in the lobby
// drops, then disconnect the call automatically.

void Voice::AddChangeHandler(std::function<void()> handler) {
  change_handlers_.push_back(std::move(handler));
}

void Voice::OnChange() const {
  for (const auto& handler : change_handlers_) {
    handler();
  }
}

std::optional<discordpp::Call> Voice::GetCall(const uint64_t user_id) const {
  const auto entry = active_calls_.find(user_id);
  if (entry != active_calls_.end()) {
    return entry->second;
  }
  return std::nullopt;
}

}  // namespace discord_social_tui