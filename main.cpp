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

#include <iostream>
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

int main() {
  // Sample data for the list
  std::vector<std::string> list_items = {
      "👋 Jane", "👋 Alex", "👋 Amy", "💤 Daria", "💤 Greg",
  };

  // Left side menu component
  int selected_index = 0;
  auto menu =
      ftxui::Menu(&list_items, &selected_index, ftxui::MenuOption::Vertical());

  // Action buttons
  bool profile_selected = false;
  bool dm_selected = false;
  bool voice_selected = false;

  auto profile_button =
      ftxui::Button("Profile", [&] { profile_selected = true; });
  auto dm_button = ftxui::Button("Message", [&] { dm_selected = true; });
  auto voice_button = ftxui::Button("Voice", [&] { voice_selected = true; });

  // Button row for the top of the right panel
  auto button_row =
      ftxui::Container::Horizontal({profile_button, dm_button, voice_button});

  // Content container with button row and content area
  auto content_container = ftxui::Container::Vertical({
      button_row,
      ftxui::Renderer([] {
        return ftxui::vbox(
            {ftxui::separator(),
             ftxui::paragraph("This area will display the content of the "
                              "selected channel.")});
      }),
  });

  // Wrap in renderer for the right panel
  auto content = ftxui::Renderer(content_container,
                                 [&] { return content_container->Render(); });

  // Horizontal layout with the constrained menu
  int left_width = 20;
  auto container = ftxui::ResizableSplitLeft(menu, content, &left_width);

  auto screen = ftxui::ScreenInteractive::Fullscreen();
  screen.Loop(container);

  return EXIT_SUCCESS;
}