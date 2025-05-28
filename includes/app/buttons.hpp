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

#include <ftxui/component/component_base.hpp>
#include <memory>

#include "friend.hpp"
#include "voice.hpp"

namespace discord_social_tui {

/// This class manages the buttons for the application, such as profile, DMs
/// voice, and disconnect buttons.
class Buttons {
 public:
  explicit Buttons(const std::shared_ptr<Friends>& friends,
                   const std::shared_ptr<Voice>& voice);
  [[nodiscard]] ftxui::Component Render() const;

 private:
  std::shared_ptr<Friends> friends_;
  std::shared_ptr<Voice> voice_;
  ftxui::Component disconnect_button_;
  ftxui::Component voice_button_;
  ftxui::Component profile_button_;
  ftxui::Component dm_button_;
};

}  // namespace discord_social_tui