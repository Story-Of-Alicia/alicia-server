/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#ifndef MISSION_REGISTRY_HPP
#define MISSION_REGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <optional>
#include <unordered_map>

namespace server::registry
{

//! Mission info data loaded from missions.yaml.
struct Mission
{
  uint16_t id{};
  uint16_t preMissionId{};
  uint16_t nextMissionId{};
  uint16_t mapId{};
  uint32_t minLevel{};
  uint32_t maxLevel{};
  uint32_t waitingTime{};
  uint32_t reward{};
  bool closed{};
};

class MissionRegistry
{
public:
  void ReadConfig(const std::filesystem::path& configPath);
  [[nodiscard]] std::optional<Mission> GetMission(uint16_t id) const;
  [[nodiscard]] const std::unordered_map<uint16_t, Mission>& GetMissions() const;

private:
  std::unordered_map<uint16_t, Mission> _missions{};
};

} // namespace server::registry

#endif // MISSION_REGISTRY_HPP
