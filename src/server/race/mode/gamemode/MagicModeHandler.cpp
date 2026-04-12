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

namespace server::race::mode
{

MagicGameMode::MagicGameMode(RaceDirector& director, RaceDirector::RaceInstance& raceInstance)
  : GameModeHandler(director, raceInstance)
{}

MagicGameMode::~MagicGameMode() = default;

void MagicGameMode::OnHurdleClear(
  ClientId clientId,
  const protocol::AcCmdCRHurdleClearResult& command)
{
  const auto& clientContext = _director.GetClientContext(clientId);

  auto& racer = _raceInstance.tracker.GetRacer(
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
        _gameModeInfo.perfectJumpMaxBonusCombo,
        racer.jumpComboValue);
      // Calculate max combo count * perfect jump boost unit points
      const auto& gainedStarPointsFromCombo = applicableComboCount * _gameModeInfo.perfectJumpUnitStarPoints;
      // Add boost points to character boost tracker
      racer.starPointValue = std::min(
        racer.starPointValue + _gameModeInfo.perfectJumpStarPoints + gainedStarPointsFromCombo,
        _gameModeInfo.starPointsMax);

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

      uint32_t gainedStarPoints = _gameModeInfo.goodJumpStarPoints;
      if (racer.gaugeBuff) {
        // TODO: Something sensible, idk what the bonus does
        gainedStarPoints *= 2;
      }

      // Increment boost gauge by a good jump
      racer.starPointValue = std::min(
        racer.starPointValue + gainedStarPoints,
        _gameModeInfo.starPointsMax);

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
    _raceInstance.raceGameMode == protocol::GameMode::Magic &&
    racer.starPointValue >= _gameModeInfo.starPointsMax &&
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
  ClientId clientId,
  const protocol::AcCmdUserRaceUpdatePos& command)
{
  // Handle general command functionality (item spawning)
  GameModeHandler::OnRaceUserPos(clientId, command);

  const auto& clientContext = _director.GetClientContext(clientId);
  auto& racer = _raceInstance.tracker.GetRacer(clientContext.characterUid);

  // Only regenerate magic during active race (after countdown finishes)
  // Check if game mode is magic, race is active, countdown finished, and not holding an item
  const bool raceActuallyStarted = std::chrono::steady_clock::now() >= _raceInstance.raceStartTimePoint;

  if (_raceInstance.raceGameMode == protocol::GameMode::Magic
    && racer.state == tracker::RaceTracker::Racer::State::Racing
    && raceActuallyStarted
    && not racer.magicItem.has_value())
  {
    if (racer.starPointValue < _gameModeInfo.starPointsMax)
    {
      // TODO: add these to configuration somewhere
      // Eyeballed these values from watching videos
      constexpr uint32_t NoItemHeldBoostAmount = 2000;
      // TODO: does holding an item and with certain equipment give you magic? At a reduced rate?
      constexpr uint32_t ItemHeldWithEquipmentBoostAmount = 1000;
      uint32_t gainedStarPoints;
      if (racer.magicItem.has_value()) {
        gainedStarPoints = ItemHeldWithEquipmentBoostAmount;
      } else {
        gainedStarPoints = NoItemHeldBoostAmount;
      }
      if (racer.gaugeBuff) {
        // TODO: Something sensible, idk what the bonus does
        gainedStarPoints *= 2;
      }
      racer.starPointValue = std::min(_gameModeInfo.starPointsMax, racer.starPointValue + gainedStarPoints);
    }

    // Conditional already checks if there is no magic item and gamemode is magic,
    // only check if racer has max magic gauge to give magic item
    protocol::AcCmdCRStarPointGetOK starPointResponse{
      .characterOid = command.oid,
      .starPointValue = racer.starPointValue,
      .giveMagicItem = racer.starPointValue >= _gameModeInfo.starPointsMax
    };

    _director.GetCommandServer().QueueCommand<decltype(starPointResponse)>(
      clientId,
      [starPointResponse]
      {
        return starPointResponse;
      });
  }

  for (const auto& raceClientId : _raceInstance.clients)
  {
    // Prevent broadcast to self.
    if (clientId == raceClientId)
      continue;
  }
}

void MagicGameMode::OnItemGet(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] const protocol::AcCmdUserRaceItemGet& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnRequestSpur(
  ClientId,
  const protocol::AcCmdCRRequestSpur&)
{
  // Ignore request spur (speed) in magic mode
}

void MagicGameMode::OnStartingRate(
  ClientId clientId,
  const protocol::AcCmdCRStartingRate& command)
{
  // TODO: check for sensible values
  if (command.unk1 < 1 && command.boostGained < 1)
  {
    // Velocity and boost gained is not valid
    // TODO: throw?
    return;
  }

  const auto& clientContext = _director.GetClientContext(clientId);
  auto& racer = _raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  // TODO: validate boost gained against a table and determine good/perfect start
  racer.starPointValue = std::min(
    racer.starPointValue + command.boostGained,
    _gameModeInfo.starPointsMax);

  // Only send this on good/perfect starts
  protocol::AcCmdCRStarPointGetOK response{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false // TODO: this would never give a magic item on race start, right?
  };

  _director.GetCommandServer().QueueCommand<decltype(response)>(
    clientId,
    [clientId, response]()
    {
      return response;
    });
}

void MagicGameMode::OnUseMagicItem(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] const protocol::AcCmdCRUseMagicItem& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

} // namespace server::race::mode
