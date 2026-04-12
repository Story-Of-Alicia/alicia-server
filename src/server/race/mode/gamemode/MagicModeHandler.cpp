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
  ClientId clientId,
  const protocol::AcCmdUserRaceItemGet& command)
{
  const auto& clientContext = _director.GetClientContext(clientId);
  auto& item = _raceInstance.tracker.GetItems().at(command.itemId);

  constexpr auto ItemRespawnDuration = std::chrono::milliseconds(500);
  item.respawnTimePoint = std::chrono::steady_clock::now() + ItemRespawnDuration;

  auto& racer = _raceInstance.tracker.GetRacer(clientContext.characterUid);

  // Magic items should respawn at a near-instant rate
  item.respawnTimePoint = std::chrono::steady_clock::now();

  uint32_t magicItem{};
  if (not racer.magicItem.has_value())
  {
    // Racer is empty handed

    // Get the item type of the picked up item (408, 409 etc)
    const uint32_t magicItemType = item.currentType;

    // Get the magic slot index to indicate to the racer that they
    // have the item (water shield, ice wall etc).
    magicItem = _director.GetServerInstance().GetCourseRegistry()
      .GetItemTypeInfo(magicItemType).magicSlot;

    // Response with OK to the client that they have a new item in hand
    protocol::AcCmdCRRequestMagicItemOK magicItemOk{
      .characterOid = command.characterOid,
      .magicItemId = racer.magicItem.emplace(magicItem),
      .member3 = 0};

    _director.GetCommandServer().QueueCommand<decltype(magicItemOk)>(
      clientId,
      [clientId, magicItemOk]()
      {
        return magicItemOk;
      });
  }
  else
  {
    // Racer is already holding the item, do not replace it
    magicItem = racer.magicItem.value();
  }

  // Now that the magic item on the ground has been picked up,
  // randomly pick the new item type for this picked up item
  if (!item.itemTypes.empty())
  {
    static std::random_device rd;
    std::uniform_int_distribution<size_t> distribution(0, item.itemTypes.size() - 1);
    item.currentType = item.itemTypes[distribution(rd)];
  }
  else
  {
    // TODO: Item types is empty, use deck ID instead?
  }

  // Notify racers in the race room that the invoking racer is now
  // holding a new magic item
  protocol::AcCmdCRRequestMagicItemNotify notify{
    .magicItemId = racer.magicItem.emplace(magicItem),
    .characterOid = command.characterOid,
  };

  for (const ClientId& roomClientId : _raceInstance.clients)
  {
    // Prevent self broadcast,
    // this prevents the double pickup UI bug for the invoker)
    if (roomClientId == clientId)
      continue;

    _director.GetCommandServer().QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]()
      {
        return notify;
      });
  }

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
  ClientId clientId,
  const protocol::AcCmdCRUseMagicItem& command)
{
  const auto& clientContext = _director.GetClientContext(clientId);
  auto& racer = _raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Client tried to perform action on behalf of different racer");
    return;
  }
  const uint16_t effectInstanceId = _raceInstance.tracker.GetNextEffectInstanceIdAndIncrementBy(1);

  auto targetList = command.targetList;

  auto magicSlotInfo = _director.GetServerInstance().GetMagicRegistry().GetSlotInfo(command.magicItemId);

  //TODO: Remove the crit chance imediatly after throwing the crit effect.
  if (racer.critChance && (magicSlotInfo.criticalType != 0))
  {
    magicSlotInfo = _director.GetServerInstance().GetMagicRegistry().GetSlotInfo(magicSlotInfo.criticalType);
  }

  // Darkfire should only affect one target
  // Client sends all targets infront of them but we should only apply the effect to the targeted one (the arrow above their head)
  if (magicSlotInfo.type == 14)
    targetList.resize(1);

  protocol::AcCmdCRUseMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = magicSlotInfo.type,
    .iceWallProperties = command.iceWallProperties,
    .targetList = targetList,
    .effectInstanceId = effectInstanceId,
    .unk4 = magicSlotInfo.castingTime
  };

  _director.GetCommandServer().QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });

  // Notify other players that this player used their magic item
  protocol::AcCmdCRUseMagicItemNotify usageNotify{
    .characterOid = command.characterOid,
    .magicItemId = magicSlotInfo.type,
    .iceWallProperties = command.iceWallProperties,
    .targetList = targetList,
    .effectInstanceId = effectInstanceId,
    .unk4 = magicSlotInfo.castingTime
  };

  // Send usage notification to other players
  for (const auto& raceClientId : _raceInstance.clients)
  {
    if (raceClientId == clientId)
      continue;

    _director.GetCommandServer().QueueCommand<decltype(usageNotify)>(
      raceClientId,
      [usageNotify]() { return usageNotify; });
  }


  // Send effect for items that have instant effects
  std::vector<std::tuple<server::tracker::Oid, std::optional<std::function<void()>>>> affectedOidsAndAfterEffectCallbacks;
  switch (magicSlotInfo.type)
  {
    // Shield
    // TODO: Maybe not change it if they already have a stronger shield?
    case 4:
    {
      const auto afterEffectRemoved = [&racer]()
      {
        racer.shield = tracker::RaceTracker::Racer::Shield::None;
      };
      _director.ScheduleSkillEffect(_raceInstance, command.characterOid, racer.oid, magicSlotInfo, afterEffectRemoved, effectInstanceId);
      racer.shield = tracker::RaceTracker::Racer::Shield::Normal;
      break;
    }
    case 5:
    {
      const auto afterEffectRemoved = [&racer]()
      {
        racer.shield = tracker::RaceTracker::Racer::Shield::None;
      };
      _director.ScheduleSkillEffect(_raceInstance, command.characterOid, racer.oid, magicSlotInfo, afterEffectRemoved, effectInstanceId);
      racer.shield = tracker::RaceTracker::Racer::Shield::Critical;
      break;
    }
    // Booster
    case 6:
    case 7:
    {
      _director.ScheduleSkillEffect(_raceInstance, command.characterOid, racer.oid, magicSlotInfo, std::nullopt, effectInstanceId);
      break;
    }
    // Phoenix
    case 8:
    case 9:
    {
      const auto afterEffectRemoved = [&racer]()
      {
        racer.hotRodded = false;
      };
      _director.ScheduleSkillEffect(_raceInstance, command.characterOid, racer.oid, magicSlotInfo, afterEffectRemoved, effectInstanceId);
      racer.hotRodded = true;
      break;
    }
    // IceWall
    case 10:
    case 11:
    {
      const uint16_t obstacleInstanceCount = static_cast<uint16_t>(command.targetList.size());
      if (obstacleInstanceCount > 1)
      {
        // If its a crit ice wall, add the 2 missing InstanceIds to the tracker so that they can be used for the breakdown and expiration of the effect
        _raceInstance.tracker.GetNextEffectInstanceIdAndIncrementBy(obstacleInstanceCount - 1);
      }
      auto magicExpire = protocol::AcCmdRCMagicExpire{
        .magicType = magicSlotInfo.type,
        .firstObstacleInstanceId = effectInstanceId,
        .obstacleInstanceCount = obstacleInstanceCount,
        .breakdown = 0
      };
      _director.GetScheduler().Queue(
        [this, magicExpire]()
        {
          for (const ClientId& raceClientId : _raceInstance.clients)
          {
            _director.GetCommandServer().QueueCommand<decltype(magicExpire)>(
              raceClientId,
              [magicExpire]() { return magicExpire; });
          }
        },
        Scheduler::Clock::now() + std::chrono::seconds(4)); // TODO: Change to 4 seconds
      break;
    }
    // BufPower
    case 20:
    case 21:
    {
      for (auto& otherRacer : _raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          otherRacer.critChance = true;
          const auto afterEffectRemoved = [&otherRacer]()
          {
            otherRacer.critChance = false;
          };
          _director.ScheduleSkillEffect(_raceInstance, command.characterOid, otherRacer.oid, magicSlotInfo, afterEffectRemoved, effectInstanceId);
        }
      }
      break;
    }
    // BufGauge
    case 22:
    case 23:
    {
      for (auto& otherRacer : _raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          otherRacer.gaugeBuff = true;
          const auto afterEffectRemoved = [&otherRacer]()
          {
            otherRacer.gaugeBuff = false;
          };
          _director.ScheduleSkillEffect(_raceInstance, command.characterOid, otherRacer.oid, magicSlotInfo, afterEffectRemoved, effectInstanceId);
        }
      }
      break;
    }
    // BufSpeed
    case 24:
    case 25:
    {
      for (auto& otherRacer : _raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          _director.ScheduleSkillEffect(_raceInstance, command.characterOid, otherRacer.oid, magicSlotInfo, std::nullopt, effectInstanceId);
        }
      }
      break;
    }
  }

  racer.magicItem.reset();
}

} // namespace server::race::mode
