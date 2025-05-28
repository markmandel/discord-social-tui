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

#include "app/buttons.hpp"

#include <spdlog/spdlog.h>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>

namespace discord_social_tui {

Buttons::Buttons(const std::shared_ptr<Friends>& friends,
                 const std::shared_ptr<Voice>& voice)
    : friends_(friends), voice_(voice) {
  // Initialize button components
  profile_button_ =
      ftxui::Button("Profile", [] { SPDLOG_INFO("pressed profile button"); });

  dm_button_ = ftxui::Button("Message", [] {
    // TODO: Handle DM selection
  });

  voice_button_ = ftxui::Button("Voice", [this] {
    friends_->GetSelectedFriend().and_then(
        [&](const auto& friend_) -> std::optional<std::monostate> {
          this->voice_->Call(friend_);
          return std::monostate{};
        });
    voice_button_->Detach();
    horizontal_container_->Add(disconnect_button_);
  });

  disconnect_button_ =
      ftxui::Button("🔉 Disconnect", [] { SPDLOG_INFO("Hung Up!"); });

  horizontal_container_ = ftxui::Container::Horizontal(
      {profile_button_, dm_button_, voice_button_});
}

const ftxui::Component& Buttons::GetComponent() const {
  return horizontal_container_;
}

}  // namespace discord_social_tui