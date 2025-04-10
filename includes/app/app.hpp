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
#include <mutex>
#include <string>
#include <vector>

#include "app/friend.hpp"
#include "discordpp.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

namespace discord_social_tui {

class App {
 public:
  // Constructor with application ID and client
  App(uint64_t application_id, std::shared_ptr<discordpp::Client> client);

  // Run the application
  int Run();

 private:
  // Width of the left menu
  static constexpr int LEFT_WIDTH = 20;

  // Friends list
  std::unique_ptr<Friends> friends_;

  // Application configuration
  uint64_t application_id_;

  // Discord client
  std::shared_ptr<discordpp::Client> client_;

  // Components
  int left_width_;
  bool profile_selected_;
  bool dm_selected_;
  bool voice_selected_;

  ftxui::Component menu_;
  ftxui::Component container_;
  ftxui::ScreenInteractive screen_;
  bool show_authenticating_modal_;

  // Flag to ensure Ready() is only called once
  std::once_flag ready_flag_;

  [[nodiscard]] ftxui::Component AuthenticatingModal(
      const ftxui::Component &main) const;
  void StartStatusChangedCallback();
  void Ready();
  void Authorize();
  void Presence() const;
  void StartFriends() const;
};

}  // namespace discord_social_tui