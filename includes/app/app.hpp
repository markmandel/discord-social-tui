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
#include <vector>

#include "discordpp.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

namespace discord_social_tui {

class App {
 public:
  // Constructor with application ID and client
  App(std::string application_id, std::shared_ptr<discordpp::Client> client);

  // Run the application
  int Run();

 private:
  // Width of the left menu
  static constexpr int LEFT_WIDTH = 20;

  // Sample data for the list
  std::vector<std::string> list_items_;

  // Application configuration
  std::string application_id_;

  // Discord client
  std::shared_ptr<discordpp::Client> client_;

  // Components
  int selected_index_;
  bool profile_selected_;
  bool dm_selected_;
  bool voice_selected_;
  int left_width_;

  ftxui::Component menu_;
  ftxui::Component content_container_;
  ftxui::Component container_;
  ftxui::ScreenInteractive screen_;
};

}  // namespace discord_social_tui