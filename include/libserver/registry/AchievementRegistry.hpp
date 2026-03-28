/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#ifndef ACHIEVEMENTREGISTRY_HPP
#define ACHIEVEMENTREGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct AchievementInfo
{
  //! Achievement TID. References libconfig `Achievements` table.
  uint16_t tid{};
  //! Event ID that triggers progress for this achievement.
  uint16_t eventId{};
  //! Condition function name. "TRUE" means any event with matching eventId counts.
  std::string function{};
  //! Progress threshold required for completion.
  uint32_t successValue{};
  //! Which achievement book this belongs to (1-8).
  uint8_t bookType{};
  //! Carrot reward on completion.
  uint32_t reward{};
  //! Achievement points awarded on completion.
  uint32_t points{};
};

class AchievementRegistry
{
public:
  AchievementRegistry();

  void ReadConfig(const std::filesystem::path& configPath);

  //! Get all achievements triggered by a given event ID.
  [[nodiscard]] std::vector<const AchievementInfo*>
  GetAchievementsByEvent(uint16_t eventId) const;

  //! Get a specific achievement by TID.
  [[nodiscard]] const AchievementInfo* GetAchievement(uint16_t tid) const;

private:
  //! All achievements keyed by TID.
  std::unordered_map<uint16_t, AchievementInfo> _achievements;
  //! Index: event ID -> list of achievement TIDs.
  std::unordered_multimap<uint16_t, uint16_t> _eventToTids;
};

} // namespace server::registry

#endif // ACHIEVEMENTREGISTRY_HPP
