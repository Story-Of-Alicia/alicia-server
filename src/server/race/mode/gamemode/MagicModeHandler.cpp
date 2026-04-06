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

#include "server/race/mode/gamemode/MagicModeHandler.hpp"
#include "server/ServerInstance.hpp"

namespace server::race::mode
{

void MagicGameMode::OnHurdleClear(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdCRHurdleClearResult& command)
{
  const auto& clientContext = _director.GetClientContext(clientId);

  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  protocol::AcCmdCRHurdleClearResultOK response{
    .characterOid = command.characterOid,
    .hurdleClearType = command.hurdleClearType,
    .jumpCombo = 0,
    .unk3 = 0
  };

  // Give magic item is calculated later
  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  const auto& gameModeTemplate = _director.GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
      static_cast<uint8_t>(protocol::GameMode::Magic));

  switch (command.hurdleClearType)
  {
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect:
    {
      // Perfect jump over the hurdle.
      racer.jumpComboValue = std::min(
        static_cast<uint32_t>(99),
        racer.jumpComboValue + 1);

      // Calculate max applicable combo
      const auto& applicableComboCount = std::min(
        gameModeTemplate.perfectJumpMaxBonusCombo,
        racer.jumpComboValue);
      // Calculate max combo count * perfect jump boost unit points
      const auto& gainedStarPointsFromCombo = applicableComboCount * gameModeTemplate.perfectJumpUnitStarPoints;
      // Add boost points to character boost tracker
      racer.starPointValue = std::min(
        racer.starPointValue + gameModeTemplate.perfectJumpStarPoints + gainedStarPointsFromCombo,
        gameModeTemplate.starPointsMax);

      // Update boost gauge
      starPointResponse.starPointValue = racer.starPointValue;
      break;
    }
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Good:
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::DoubleJumpOrGlide:
    {
      // Not a perfect jump over the hurdle, reset the jump combo.
      racer.jumpComboValue = 0;
      response.jumpCombo = racer.jumpComboValue;

      uint32_t gainedStarPoints = gameModeTemplate.goodJumpStarPoints;
      if (racer.gaugeBuff) {
        // TODO: Something sensible, idk what the bonus does
        gainedStarPoints *= 2;
      }

      // Increment boost gauge by a good jump
      racer.starPointValue = std::min(
        racer.starPointValue + gainedStarPoints,
        gameModeTemplate.starPointsMax);

      // Update boost gauge
      starPointResponse.starPointValue = racer.starPointValue;
      break;
    }
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Collision:
    {
      // A collision with hurdle, reset the jump combo.
      racer.jumpComboValue = 0;
      response.jumpCombo = racer.jumpComboValue;
      break;
    }
    default:
    {
      spdlog::warn("Unhandled hurdle clear type {}",
        static_cast<uint8_t>(command.hurdleClearType));
      return;
    }
  }

  // Needs to be assigned after hurdle clear result calculations
  // Triggers magic item request when set to true (if gamemode is magic and magic gauge is max)
  // TODO: is there only perfect clears in magic race?
  starPointResponse.giveMagicItem =
    raceInstance.raceGameMode == protocol::GameMode::Magic &&
    racer.starPointValue >= gameModeTemplate.starPointsMax &&
    command.hurdleClearType == protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect;

  // Update the star point value if the jump was not a collision.
  if (command.hurdleClearType != protocol::AcCmdCRHurdleClearResult::HurdleClearType::Collision)
  {
    _director.GetCommandServer().QueueCommand<decltype(starPointResponse)>(
      clientId,
      [clientId, starPointResponse]()
      {
        return starPointResponse;
      });
  }

  _director.GetCommandServer().QueueCommand<decltype(response)>(
    clientId,
    [clientId, response]()
    {
      return response;
    });
}

void MagicGameMode::OnRaceUserPos(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] const protocol::AcCmdUserRaceUpdatePos& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnItemGet(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] const protocol::AcCmdUserRaceItemGet& command,
  [[maybe_unused]] tracker::RaceTracker::Item& item)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnRequestSpur(
  ClientId,
  RaceDirector::RaceInstance&,
  const protocol::AcCmdCRRequestSpur&)
{
  // Ignore request spur (speed) in magic mode
}

void MagicGameMode::OnStartingRate(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] const protocol::AcCmdCRStartingRate& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnUseMagicItem(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] const protocol::AcCmdCRUseMagicItem& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

} // namespace server::race::mode
