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

#ifndef SPEEDREGISTRY_HPP
#define SPEEDREGISTRY_HPP

#include <libserver/registry/Registry.hpp>

#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct TeamSpurGaugeInfo
{
  uint32_t racerCount{};
  float teamSpurMax{};
  float winTeamSpurConsumeRate{};
  float loseTeamSpurConsumeRate{};
  std::vector<float> points;
  std::vector<float> fillRates;
  uint32_t reduceWaitTime{};
  uint32_t reduceDelay{};
  float reduceRate{};
  uint32_t goodTiming{};
};

class SpeedRegistry : public Registry
{
public:
  void ReadConfig(const std::filesystem::path& configPath) override;
  void Clear() override;

  //! Returns the TeamSpurGaugeInfo for a given racer count.
  //! @throws std::runtime_error if racerCount is not found in the registry.
  [[nodiscard]] const TeamSpurGaugeInfo& GetTeamSpurGaugeInfo(uint32_t racerCount) const;

private:
  std::unordered_map<uint32_t, TeamSpurGaugeInfo> _teamSpurGaugeInfoMap;
};

} // namespace server::registry

#endif // SPEEDREGISTRY_HPP
