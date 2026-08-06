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

#ifndef FESTIVALREGISTRY_HPP
#define FESTIVALREGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct FestivalSettings
{
  uint32_t triggerChance{};
  std::optional<uint32_t> forcedMissionType{};
};

struct FestivalMission
{
  uint32_t type{};
  std::string name{};
  std::string option{};
  bool clientCheck{};
  bool serverCheck{};
  float clientFailCheckTime{};
  uint32_t playEvent{};
  std::string eventValue{};
  uint32_t eventCompare{};
  uint32_t gameMode{};
  uint32_t teamMode{};
  std::vector<uint32_t> excludedMaps{};
};

class FestivalRegistry
{
public:
  void ReadConfig(const std::filesystem::path& configPath);

  [[nodiscard]] const FestivalSettings& GetSettings() const;
  [[nodiscard]] const FestivalMission* GetMission(uint32_t type) const;
  [[nodiscard]] const std::unordered_map<uint32_t, FestivalMission>& GetMissions() const;

private:
  FestivalSettings _settings{};
  std::unordered_map<uint32_t, FestivalMission> _missions{};
};

} // namespace server::registry

#endif // FESTIVALREGISTRY_HPP
