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
#include <vector>
#include <string>
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

int main() {
    // Sample data for the list
    std::vector<std::string> list_items = {
        "Channel 1",
        "Channel 2",
        "Channel 3",
        "Channel 4",
        "Channel 5",
        "Direct Messages",
    };
    
    int selected_idx = 0;
    
    // Left side menu component
    auto menu = ftxui::Menu(&list_items, &selected_idx, ftxui::MenuOption::Vertical());
    
    // Dummy content for the right side
    auto content = ftxui::Renderer([] {
        return ftxui::vbox({
            ftxui::text("Welcome to Discord Social TUI"),
            ftxui::text("Select a channel from the left panel"),
            ftxui::separator(),
            ftxui::paragraph("This area will display the content of the selected channel.")
        });
    });
    
    // Horizontal layout with a fixed size for the left panel
    auto container = ftxui::ResizableSplitLeft(
        menu,
        content,
        &selected_idx
    );
    
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    screen.Loop(container);
    
    return 0;
}