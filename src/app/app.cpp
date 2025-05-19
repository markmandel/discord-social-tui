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

#include "app/app.hpp"

#include <spdlog/spdlog.h>

#include <optional>
#include <utility>

#include "app/friend.hpp"
#include "app/profile.hpp"
#include "ftxui/component/loop.hpp"
#include "ftxui/dom/elements.hpp"

namespace discord_social_tui {

// Constructor for the App class
App::App(const uint64_t application_id,
         std::shared_ptr<discordpp::Client> client)
    : friends_{std::make_shared<Friends>()},
      application_id_{application_id},
      client_{std::move(client)},
      voice_{std::make_unique<Voice>(client)},
      left_width_{LEFT_WIDTH},
      dm_selected_{false},
      screen_{ftxui::ScreenInteractive::Fullscreen()},
      show_authenticating_modal_{false},
      profile_{std::make_unique<Profile>(friends_)} {
  // Log the application ID
  spdlog::debug("App initialized with Discord Application ID: {}",
                application_id_);

  // Left side menu component - use Friends' internal selection index
  menu_ = ftxui::Menu(friends_.get(), friends_->GetSelectedIndex(),
                      ftxui::MenuOption::Vertical());

  // Action buttons
  auto profile_button =
      ftxui::Button("Profile", [&] { spdlog::info("pressed profile button"); });
  auto dm_button = ftxui::Button("Message", [&] { dm_selected_ = true; });

  auto voice_button = ftxui::Button("Voice", [&] {
    auto friend_ = friends_->GetSelectedFriend()
                          .and_then([&](const auto& friend_) -> std::optional<std::monostate> {
                            this->voice_->Call(friend_);
                            return std::monostate{};
                          });
  });

  // Button row for the top of the right panel
  auto button_row =
      ftxui::Container::Horizontal({profile_button, dm_button, voice_button});

  // Content container with button row and content area
  const auto content = ftxui::Container::Vertical({
      button_row,
      profile_->Render(),
  });

  // Horizontal layout with the constrained menu
  container_ = ftxui::ResizableSplitLeft(menu_, content, &left_width_);
  // Wrap main container with loading modal
  container_ = AuthenticatingModal(container_);
}

// Create a modal for when we are authenticating
ftxui::Component App::AuthenticatingModal(const ftxui::Component& main) const {
  constexpr int MODAL_WIDTH = 100;
  constexpr int MODAL_HEIGHT = 30;
  // Create a simple loading message with a border
  const auto loading_content = ftxui::Renderer([] {
    return ftxui::vbox({ftxui::text("🔗 Authenticating...") | ftxui::center}) |
           ftxui::vcenter |
           ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, MODAL_WIDTH) |
           ftxui::size(ftxui::HEIGHT, ftxui::GREATER_THAN, MODAL_HEIGHT) |
           ftxui::center | ftxui::border;
  });

  // Modal component
  return ftxui::Modal(main, loading_content, &show_authenticating_modal_);
}

void App::StartStatusChangedCallback() {
  client_->SetStatusChangedCallback([&](const discordpp::Client::Status status,
                                        const discordpp::Client::Error error,
                                        int32_t errorDetail) {
    spdlog::debug("Social SDK Status Change: {}",
                  discordpp::Client::StatusToString(status));

    if (error != discordpp::Client::Error::None) {
      spdlog::error("Social SDK Status Error: {}, Details: {}",
                    discordpp::Client::ErrorToString(error), errorDetail);
    }

    if (status == discordpp::Client::Status::Ready) {
      // Use call_once to ensure Ready() is only called once
      std::call_once(ready_flag_, [this]() { Ready(); });
    }
  });
}

// Function to set up the application once we're authenticated.
void App::Ready() {
  // Hide the modal
  show_authenticating_modal_ = false;

  // Set up rich presence
  Presence();
  StartFriends();
}

// Function to initialize friends and track their status changes
void App::StartFriends() const {
  // In a real application, these would be actual UserHandles from the Discord
  // SDK For now, we just log that this would happen here
  spdlog::info("Initializing friends...");

  for (auto& relationship : client_->GetRelationships()) {
    relationship.User().and_then(
        [&](const auto& user) -> std::optional<discordpp::UserHandle> {
          // Log information about the friend we're adding
          spdlog::debug("Adding friend: {} (ID: {})", user.Username(),
                        user.Id());

          // Only show if a real friend.
          if (relationship.DiscordRelationshipType() ==
                  discordpp::RelationshipType::Friend ||
              relationship.GameRelationshipType() ==
                  discordpp::RelationshipType::Friend) {
            friends_->AddFriend(std::make_shared<Friend>(user));
          }
          return std::nullopt;
        });
  }

  // resort friends when their status updates
  client_->SetUserUpdatedCallback(
      [&](uint64_t userId) { friends_->SortFriends(); });

  client_->SetRelationshipCreatedCallback(
      [&](uint64_t userId, bool isDiscordRelationshipUpdate) {
        client_->GetUser(userId).and_then(
            [&](const auto& user) -> std::optional<discordpp::UserHandle> {
              spdlog::debug("🔥 Relationship created: {} (ID: {})",
                            user.Username(), user.Id());

              auto const relationship = user.Relationship();
              // Only show if a real friend
              if (relationship.DiscordRelationshipType() ==
                      discordpp::RelationshipType::Friend ||
                  relationship.GameRelationshipType() ==
                      discordpp::RelationshipType::Friend) {
                friends_->AddFriend(std::make_shared<Friend>(user));
              }

              friends_->AddFriend(std::make_shared<Friend>(user));
              return std::nullopt;
            });
      });

  client_->SetRelationshipDeletedCallback(
      [&](uint64_t userId, bool isDiscordRelationshipUpdate) {
        spdlog::debug("🔥 Relationship deleted (ID: {})", userId);
        friends_->RemoveFriend(userId);
      });
}

// Set rich presence for the Discord client
void App::Presence() const {
  // Configure rich presence details
  discordpp::Activity activity;
  activity.SetType(discordpp::ActivityTypes::Playing);
  activity.SetState("Discord on the Command Line");
  activity.SetDetails("Better TUI than me...");

  // Update rich presence
  spdlog::debug("Updating Discord rich presence...");
  client_->UpdateRichPresence(
      activity, [](const discordpp::ClientResult& result) {
        if (result.Successful()) {
          spdlog::info("Rich Presence updated successfully");
        } else {
          spdlog::error("Rich Presence update failed: {}", result.Error());
        }
      });
}

void App::Authorize() {
  // Show the authenticating modal while we're authorizing
  show_authenticating_modal_ = true;

  // Generate OAuth2 code verifier for authentication
  auto code_verifier = client_->CreateAuthorizationCodeVerifier();

  // Set up authentication arguments
  discordpp::AuthorizationArgs args{};
  args.SetClientId(application_id_);
  args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
  args.SetCodeChallenge(code_verifier.Challenge());

  // Begin authentication process
  client_->Authorize(args, [this, code_verifier](
                               const discordpp::ClientResult& result,
                               const std::string& code,
                               const std::string& redirect_uri) {
    if (!result.Successful()) {
      spdlog::error("Authorization failed: {}", result.Error());
      show_authenticating_modal_ = false;
      return;
    }

    spdlog::info("Authorization successful, exchanging code for token");

    // Exchange auth code for access token
    client_->GetToken(
        application_id_, code, code_verifier.Verifier(), redirect_uri,
        [this](
            const discordpp::ClientResult& result,
            const std::string&  // NOLINT(bugprone-easily-swappable-parameters)
                access_token,
            const std::string&  // NOLINT(bugprone-easily-swappable-parameters)
                refresh_token,
            const discordpp::AuthorizationTokenType token_type,
            int32_t expires_in, const std::string& scope) {
          if (!result.Successful()) {
            spdlog::error("Token exchange failed: {}", result.Error());
            return;
          }

          spdlog::info(
              "Token exchange successful, access token expires in {} seconds",
              expires_in);

          // Set the authentication token for the client
          client_->UpdateToken(token_type, std::string(access_token),
                               [this](const discordpp::ClientResult& result) {
                                 if (!result.Successful()) {
                                   spdlog::error("Token update failed: {}",
                                                 result.Error());
                                 } else {
                                   spdlog::info("Connection Social SDK...");
                                   this->client_->Connect();
                                   // The modal will be hidden when the client
                                   // is ready via the status changed callback
                                 }
                               });
        });
  });
}

// Run the application
int App::Run() {
  constexpr int SLEEP_MILLISECONDS = 10;
  StartStatusChangedCallback();

  // Start the authorization process
  Authorize();

  // Run the application loop
  ftxui::Loop loop(&screen_, container_);
  while (!loop.HasQuitted()) {
    loop.RunOnce();
    discordpp::RunCallbacks();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MILLISECONDS));
  }

  return EXIT_SUCCESS;
}

}  // namespace discord_social_tui