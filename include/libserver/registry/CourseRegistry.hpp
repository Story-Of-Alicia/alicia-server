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

#ifndef COURSEREGISTRY_HPP
#define COURSEREGISTRY_HPP

#include <cstdint>

#include <unordered_map>
#include <filesystem>

namespace server::registry
{

struct Course
{
  struct GameModeInfo
  {
    enum class Type : uint8_t
    {
      Nothing = 0,
      Speed = 1,
      Magic = 2,
      Guild = 3,
      Tutorial = 6
    };

    //! An amount of star points awarded for a good jump.
    uint32_t goodJumpStarPoints{};
    //! An amount of star points awarded for a perfect jump.
    uint32_t perfectJumpStarPoints{};
    //! A perfect jump max bonus combo.
    uint32_t perfectJumpMaxBonusCombo{};
    //! i dont know
    uint32_t perfectJumpUnitStarPoints{};
    //! i dont know
    uint32_t perfectSpurCheckTime{};
    //! An amount of points a spur consumes.
    uint32_t spurConsumeStarPoints{};
    //! A maximum amount of star points collectable.
    uint32_t starPointsMax{};
  };

  struct EventInfo
  {
  };

  struct MapBlockInfo
  {
    //! A required level to play the map.
    uint32_t requiredLevel{};
    //! A podium ID.
    uint32_t podiumId{};
    //! A fee for training on the map.
    uint32_t trainingFee{};
    //! A time limit.
    uint32_t timeLimit{};
    //! A wait time.
    uint32_t waitTime{};
  };

};

class CourseRegistry
{
public:
  CourseRegistry();

  void ReadConfig(const std::filesystem::path& configPath);

  [[nodiscard]] const Course::GameModeInfo& GetCourseGameModeInfo(
    uint8_t type);
  [[nodiscard]] const Course::MapBlockInfo& GetMapBlockInfo(
    uint32_t id);

private:
  //! A collection of game mode infos.
  std::unordered_map<uint8_t, Course::GameModeInfo> _gameModeInfo;
  //! A collection of map block infos.
  std::unordered_map<uint32_t, Course::MapBlockInfo> _mapBlockInfo;
};

}

#endif //COURSEREGISTRY_HPP
