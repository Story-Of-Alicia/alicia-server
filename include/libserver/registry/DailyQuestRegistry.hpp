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

#ifndef DAILYQUESTREGISTRY_HPP
#define DAILYQUESTREGISTRY_HPP

#include <array>
#include <cstdint>

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct DailyQuest
{
   struct DailyQuestInfo {

      enum class Type : uint8_t
      {
        NoRace = 0,
        WinSpeedTeam = 2,
        WinMagicTeam = 8,
        WinSpeedSolo = 33,
        SpeedSoloAction = 35, //perfect jumps, boosts
        WinMagicSolo = 68,
        MagicSoloAction = 76, //bolt attack
        AnyRace = 111
      };

        //! The quest id.
        uint32_t questId{};
        //! Is the quest immediately completed?
        uint32_t successType{};
        //! The amount of progress that needs to be made.
        uint32_t successValue{};
        //! Amount of EXP rewarded.
        uint32_t exp{};
        //! Amount of carrots rewarded.
        uint32_t carrots{};
      };
};

class DailyQuestRegistry
{
public:
  DailyQuestRegistry();

  void ReadConfig(const std::filesystem::path& configPath);

  [[nodiscard]] const DailyQuest::DailyQuestInfo& GetDailyQuestInfo(
    uint8_t type);

private:
  //! A collection of daily quest infos.
  std::unordered_map<uint8_t, DailyQuest::DailyQuestInfo> _dailyQuestInfo;
};

} // namespace server::registry

#endif // DAILYQUESTREGISTRY_HPP
