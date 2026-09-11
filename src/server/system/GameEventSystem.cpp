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

#include "server/system/GameEventSystem.hpp"

#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

#include <string>

namespace server
{

GameEventSystem::GameEventSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

bool GameEventSystem::IsReportedNatively(
  const registry::UserAchvEvent achievementEvent)
{
  switch (achievementEvent)
  {
    // PerfectJumpCount is already fired by the hurdle clear handler, so the client reporting it is redundant.
    case registry::UserAchvEvent::PerfectJumpCount:
      return true;

    default:
      return false;
  }
}

registry::Function GameEventSystem::ToFunction(
  const registry::UserAchvEvent achievementEvent)
{
  using Function = registry::Function;

  switch (achievementEvent)
  {
    case registry::UserAchvEvent::GlidingDistance:
      return Function::GlidingDistanceValue;

    default:
      return Function::True;
  }
}

uint32_t GameEventSystem::ParseAchievementPropertyValue(const std::string_view value)
{
  if (value.empty())
    return 0;

  try
  {
    // Floats arrive as "%.2f", so parse wide and truncate.
    const double parsed = std::stod(std::string{value});
    if (parsed <= 0.0)
      return 0;

    return static_cast<uint32_t>(parsed);
  }
  catch (const std::exception&)
  {
    spdlog::debug(
      "GameEventSystem::ParseAchievementPropertyValue: could not parse '{}'",
      value);
    return 0;
  }
}

void GameEventSystem::ReportAchievement(
  const data::Uid characterUid,
  const GameEvent::Origin origin,
  const registry::UserAchvEvent achievementEvent,
  const std::string_view achievementValue,
  const registry::GameModeFlag gameMode)
{
  if (IsReportedNatively(achievementEvent))
    return;

  _serverInstance.GetGameEventBus().Fire({
    .userAchvEvent = achievementEvent,
    .function = ToFunction(achievementEvent),
    .origin = origin,
    .characterUid = characterUid,
    .gameMode = gameMode,
    .value = ParseAchievementPropertyValue(achievementValue)});
}

registry::GameModeFlag GameEventSystem::ToGameModeFlag(
  const protocol::GameMode gameMode,
  const protocol::TeamMode teamMode)
{
  using GameModeFlag = registry::GameModeFlag;
  const bool isTeam = teamMode == protocol::TeamMode::Team;

  switch (gameMode)
  {
    case protocol::GameMode::Speed:
      return isTeam ? GameModeFlag::SpeedTeam : GameModeFlag::SpeedSoloAction;
    case protocol::GameMode::Magic:
      return isTeam ? GameModeFlag::MagicTeam : GameModeFlag::MagicSoloAction;
    default:
      return GameModeFlag::None;
  }
}

registry::GameModeFlag GameEventSystem::ToWinGameModeFlag(
  const protocol::GameMode gameMode,
  const protocol::TeamMode teamMode)
{
  using GameModeFlag = registry::GameModeFlag;

  if (teamMode == protocol::TeamMode::Team)
    return ToGameModeFlag(gameMode, teamMode);

  switch (gameMode)
  {
    case protocol::GameMode::Speed:
      return GameModeFlag::WinSpeedSolo;
    case protocol::GameMode::Magic:
      return GameModeFlag::WinMagicSolo;
    default:
      return GameModeFlag::None;
  }
}

} // namespace server
