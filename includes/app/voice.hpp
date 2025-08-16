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
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "discordpp.h"
#include "friend.hpp"

namespace discord_social_tui {

static constexpr std::string VOICE_CALL_PREFIX = "call::";

class Voice {
 public:
  explicit Voice(const std::shared_ptr<discordpp::Client> &client,
                 const std::shared_ptr<Friends> &friends)
      : client_(client), friends_(friends) {}

  /// Initiate a voice call with the currently selected friend.
  void Call();
  void Disconnect();
  /// Listen for invites and then join a voice lobby
  void Run();

  /// Add a change handler function to be called when voice state changes
  void AddChangeHandler(std::function<void()> handler);

  /// Get active voice call for the given user ID
  std::optional<discordpp::Call> GetCall(uint64_t user_id) const;

 private:
  std::shared_ptr<discordpp::Client> client_;
  std::shared_ptr<Friends> friends_;
  std::unordered_map<u_int64_t, discordpp::Call> active_calls_;
  std::vector<std::function<void()>> change_handlers_;

  /// Call all registered change handlers
  void OnChange() const;
};

}  // namespace discord_social_tui