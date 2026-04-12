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

#include "server/race/mode/gamemode/SpeedModeHandler.hpp"

namespace server::race::mode
{

SpeedGameMode::SpeedGameMode(RaceDirector& director, RaceDirector::RaceInstance& raceInstance)
  : GameModeHandler(director, raceInstance)
{}

SpeedGameMode::~SpeedGameMode() = default;

void SpeedGameMode::OnHurdleClear(
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

      // Set jump combo counter value (applies to speed)
      response.jumpCombo = racer.jumpComboValue;

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

void SpeedGameMode::OnRaceUserPos(
  ClientId clientId,
  const protocol::AcCmdUserRaceUpdatePos& command)
{
  // Base handler handles item spawning
  GameModeHandler::OnRaceUserPos(clientId, command);
}

void SpeedGameMode::OnItemGet(
  ClientId clientId,
  const protocol::AcCmdUserRaceItemGet& command)
{
  const auto& clientContext = _director.GetClientContext(clientId);
  auto& item = _raceInstance.tracker.GetItems().at(command.itemId);

  constexpr auto ItemRespawnDuration = std::chrono::milliseconds(500);
  item.respawnTimePoint = std::chrono::steady_clock::now() + ItemRespawnDuration;

  auto& racer = _raceInstance.tracker.GetRacer(clientContext.characterUid);

  switch (item.currentType)
  {
    case 101: // Gold horseshoe. Get star points until the next boost
      racer.starPointValue = std::min(((racer.starPointValue/40000)+1) * 40000, _gameModeInfo.starPointsMax);
      break;
    case 102: // Silver horseshoe. Get 10k star points
      racer.starPointValue = std::min(racer.starPointValue+10000, _gameModeInfo.starPointsMax);
      break;
    default:
      // TODO: Disconnect?
      spdlog::warn("Player {} picked up unknown item type {}",
        clientId, item.currentType);
      break;
  }

  // Only send this on good/perfect starts
  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  _director.GetCommandServer().QueueCommand<decltype(starPointResponse)>(
    clientId,
    [clientId, starPointResponse]()
    {
      return starPointResponse;
    });

  // Notify all clients in the room that this item has been picked up
  protocol::AcCmdGameRaceItemGet get{
    .characterOid = command.characterOid,
    .itemId = command.itemId,
    .itemType = item.currentType,
  };

  for (const ClientId& raceClientId : _raceInstance.clients)
  {
    _director.GetCommandServer().QueueCommand<decltype(get)>(
      raceClientId,
      [get]()
      {
        return get;
      });
  }

  // Erase the item from item instances of each client.
  for (auto& raceRacer : _raceInstance.tracker.GetRacers() | std::views::values)
  {
    raceRacer.trackedItems.erase(item.oid);
  }
}

void SpeedGameMode::OnRequestSpur(
  ClientId clientId,
  const protocol::AcCmdCRRequestSpur& command)
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

  if (racer.starPointValue < _gameModeInfo.spurConsumeStarPoints)
    throw std::runtime_error("Client is dead ass cheating (or is really desynced)");

  racer.starPointValue -= _gameModeInfo.spurConsumeStarPoints;

  protocol::AcCmdCRRequestSpurOK response{
    .characterOid = command.characterOid,
    .activeBoosters = command.activeBoosters,
    .startPointValue = racer.starPointValue,
    .comboBreak = command.comboBreak};

  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  _director._commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  _director._commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]()
    {
      return starPointResponse;
    });
}

void SpeedGameMode::OnStartingRate(
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

void SpeedGameMode::OnUseMagicItem(
  ClientId,
  const protocol::AcCmdCRUseMagicItem&)
{
  // Ignore in speed mode
}

void SpeedGameMode::OnRequestMagicItem(
  ClientId,
  const protocol::AcCmdCRRequestMagicItem&)
{
  // Ignore in speed mode
}

} // namespace server::race::mode
