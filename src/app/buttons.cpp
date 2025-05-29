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
  profile_button_ = ftxui::Button("Profile", [this] {
    SPDLOG_INFO("pressed profile button");
    OnProfileClick();
  });

  dm_button_ = ftxui::Button("Message", [this] {
    SPDLOG_INFO("pressed DM button");
    OnDMClick();
  });

  voice_button_ = ftxui::Button("🔉 Voice", [this] {
    SPDLOG_INFO("Starting voice call...");
    this->voice_->Call();
  });

  disconnect_button_ = ftxui::Button("🔇 Disconnect", [this] {
    SPDLOG_INFO("Disconnecting call!");
    voice_->Disconnect();
  });

  horizontal_container_ = ftxui::Container::Horizontal(
      {profile_button_, dm_button_, voice_button_});

  // when voice state changes, update the buttons
  voice_->AddChangeHandler([this]() { VoiceChanged(); });
}

const ftxui::Component& Buttons::GetComponent() const {
  return horizontal_container_;
}

void Buttons::VoiceChanged() const {
  const auto call = friends_->GetSelectedFriend().and_then(
      [](const auto& friend_) -> std::optional<discordpp::Call> {
        return friend_->GetVoiceCall();
      });

  if (call) {
    SPDLOG_DEBUG("Had a call!");
    if (disconnect_button_->Parent() == nullptr) {
      voice_button_->Detach();
      horizontal_container_->Add(disconnect_button_);
    }
  } else {
    SPDLOG_DEBUG("Doesn't have a call!");
    if (voice_button_->Parent() == nullptr) {
      disconnect_button_->Detach();
      horizontal_container_->Add(voice_button_);
    }
  }
}

void Buttons::AddDMClickHandler(std::function<void()> handler) {
  dm_click_handlers_.push_back(std::move(handler));
}

void Buttons::AddProfileClickHandler(std::function<void()> handler) {
  profile_click_handlers_.push_back(std::move(handler));
}

void Buttons::OnDMClick() const {
  for (const auto& handler : dm_click_handlers_) {
    handler();
  }
}

void Buttons::OnProfileClick() const {
  for (const auto& handler : profile_click_handlers_) {
    handler();
  }
}

}  // namespace discord_social_tui