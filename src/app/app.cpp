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

#include <iostream>
#include <utility>

namespace discord_social_tui {

// Constructor for the App class
App::App(const std::string& application_id,
         std::shared_ptr<discordpp::Client> client)
    : application_id_{application_id},
      client_{std::move(client)},
      list_items_{"👋 Jane", "👋 Alex", "🟣 Amy", "💤 Daria", "⚫ Greg"},
      selected_index_{0},
      profile_selected_{false},
      dm_selected_{false},
      voice_selected_{false},
      left_width_{LEFT_WIDTH},
      screen_{ftxui::ScreenInteractive::Fullscreen()} {
  // Log the application ID
  std::cout << "Discord Application ID: " << application_id_ << '\n';
  // Left side menu component
  menu_ = ftxui::Menu(&list_items_, &selected_index_,
                      ftxui::MenuOption::Vertical());

  // Action buttons
  auto profile_button =
      ftxui::Button("Profile", [&] { profile_selected_ = true; });
  auto dm_button = ftxui::Button("Message", [&] { dm_selected_ = true; });
  auto voice_button = ftxui::Button("Voice", [&] { voice_selected_ = true; });

  // Button row for the top of the right panel
  auto button_row =
      ftxui::Container::Horizontal({profile_button, dm_button, voice_button});

  // Content container with button row and content area
  content_container_ = ftxui::Container::Vertical({
      button_row,
      ftxui::Renderer([] {
        return ftxui::vbox(
            {ftxui::separator(),
             ftxui::paragraph("This area will display the content of the "
                              "selected channel.")});
      }),
  });

  // Wrap in renderer for the right panel
  auto content = ftxui::Renderer(content_container_,
                                 [&] { return content_container_->Render(); });

  // Horizontal layout with the constrained menu
  container_ = ftxui::ResizableSplitLeft(menu_, content, &left_width_);
}

// Run the application
int App::Run() {
  screen_.Loop(container_);
  return EXIT_SUCCESS;
}

}  // namespace discord_social_tui