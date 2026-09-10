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

#include "server/system/QuestSystem.hpp"

#include "server/ServerInstance.hpp"

#include <libserver/data/DataDirector.hpp>
#include <libserver/registry/QuestRegistry.hpp>

#include <spdlog/spdlog.h>

namespace server
{

QuestSystem::QuestSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
  _gameEventListenerHandle = _serverInstance.GetGameEventBus().Subscribe(
    [this](const GameEvent& event)
    {
      HandleGameEvent(event);
    });
}

bool QuestSystem::IsModeMatch(
  const registry::Quest::GameModeFlag questFlag,
  const registry::Quest::GameModeFlag eventMode)
{
  // Flag None (0): no mode restriction
  if (questFlag == registry::Quest::GameModeFlag::None)
    return true;
  // Flag Any (111): explicitly matches all race modes
  if (questFlag == registry::Quest::GameModeFlag::Any)
    return true;
  return questFlag == eventMode;
}

registry::Quest::GameModeFlag QuestSystem::ToGameModeFlag(
  const protocol::GameMode gameMode,
  const protocol::TeamMode teamMode)
{
  using GameModeFlag = registry::Quest::GameModeFlag;
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

registry::Quest::GameModeFlag QuestSystem::ToWinGameModeFlag(
  const protocol::GameMode gameMode,
  const protocol::TeamMode teamMode)
{
  using GameModeFlag = registry::Quest::GameModeFlag;

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

uint32_t QuestSystem::ToUserAchvEvent(const QuestEvent event)
{
  switch (event)
  {
    case QuestEvent::Any:
    case QuestEvent::PrizeWinner:
    case QuestEvent::RunMap:
    case QuestEvent::TeamWin:
      return 2;
    case QuestEvent::PerfectJump:
      return 29;
    case QuestEvent::FireballAttack:
      return 42;
    case QuestEvent::GlidingDistance:
      return 43;
    case QuestEvent::CollectDropItem:
      return 20;
    case QuestEvent::BoostUsed:
      return 17;
    case QuestEvent::FeedHorse:
      return 53;
    case QuestEvent::WashHorse:
      return 49;
    default:
      return 0;
  }
}

bool QuestSystem::IsEventMatch(
  const uint32_t questUserAchvEvent,
  const registry::Quest::Function function,
  const QuestEvent event,
  const uint32_t questFunctionValue,
  const uint32_t eventValue)
{
  if (questUserAchvEvent != ToUserAchvEvent(event))
    return false;

  switch (event)
  {
    case QuestEvent::Any:
      return function == registry::Quest::Function::True;
    case QuestEvent::PrizeWinner:
      return function == registry::Quest::Function::PrizeWinnerForLowLevel ||
             function == registry::Quest::Function::PrizeWinnerInMapForLowLevel;
    case QuestEvent::PerfectJump:
      return function == registry::Quest::Function::PerfectJump;
    case QuestEvent::FireballAttack:
      return function == registry::Quest::Function::FireballAttack;
    case QuestEvent::RunMap:
      return function == registry::Quest::Function::RunMap && questFunctionValue == eventValue;
    case QuestEvent::TeamWin:
      return function == registry::Quest::Function::TeamWin;
    case QuestEvent::GlidingDistance:
      return function == registry::Quest::Function::GlidingDistanceValue;
    case QuestEvent::CollectDropItem:
      return function == registry::Quest::Function::CollectDropItem;
    case QuestEvent::BoostUsed:
    case QuestEvent::FeedHorse:
    case QuestEvent::WashHorse:
      return function == registry::Quest::Function::True;
    default:
      return false;
  }
}

std::vector<protocol::AcCmdRCUpdateDailyQuestNotify> QuestSystem::OnQuestEvent(
  const data::Uid characterUid,
  const QuestEvent event,
  const registry::Quest::GameModeFlag gameMode,
  const uint32_t value)
{
  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto& questRegistry = _serverInstance.GetQuestRegistry();

  const auto characterRecord = dataDirector.GetCharacter(characterUid);
  if (!characterRecord)
  {
    spdlog::debug(
      "QuestSystem::OnQuestEvent: character {} record unavailable, ignoring event",
      characterUid);
    return {};
  }

  // Read the character's daily quest group UID
  data::Uid dailyQuestGroupUid = data::InvalidUid;
  characterRecord.Immutable([&dailyQuestGroupUid](const data::Character& character)
  {
    dailyQuestGroupUid = character.dailyQuestGroupUid();
  });

  if (dailyQuestGroupUid == data::InvalidUid)
  {
    spdlog::debug(
      "QuestSystem::OnQuestEvent: character {} has no daily quest group assigned",
      characterUid);
    return {};
  }

  auto questGroupRecord = dataDirector.GetDailyQuestGroup(dailyQuestGroupUid);
  if (!questGroupRecord)
  {
    spdlog::debug(
      "QuestSystem::OnQuestEvent: daily quest group {} for character {} unavailable",
      dailyQuestGroupUid,
      characterUid);
    return {};
  }

  std::vector<protocol::AcCmdRCUpdateDailyQuestNotify> notifies;

  questGroupRecord.Mutable([&](data::DailyQuestGroup& group)
  {
    auto questSlots = group.quests();
    for (auto& entry : questSlots)
    {
      if (entry.questId == 0)
        continue;

      const auto questDef = questRegistry.GetQuest(entry.questId);
      if (!questDef)
      {
        spdlog::warn(
          "QuestSystem::OnQuestEvent: quest {} in group {} not found in registry",
          entry.questId,
          dailyQuestGroupUid);
        continue;
      }

      // Already satisfied
      if (entry.progress >= questDef->successValue)
        continue;

      // Check game mode and function match
      if (!IsModeMatch(questDef->gameModeFlag, gameMode))
      {
        spdlog::debug(
          "QuestSystem::OnQuestEvent: quest {} mode mismatch (quest flag {}, event mode {})",
          entry.questId,
          static_cast<uint32_t>(questDef->gameModeFlag),
          static_cast<uint32_t>(gameMode));
        continue;
      }
      if (!IsEventMatch(questDef->userAchvEvent, questDef->function, event, questDef->functionValue, value))
      {
        spdlog::debug(
          "QuestSystem::OnQuestEvent: quest {} event mismatch (quest uae {}, event uae {})",
          entry.questId,
          questDef->userAchvEvent,
          ToUserAchvEvent(event));
        continue;
      }

      // Advance progress by 1 (all quest functions are count-based)
      entry.progress = std::min(entry.progress + 1, questDef->successValue);

      const bool completed = entry.progress >= questDef->successValue;

      // Determine reward params (only populated on completion)
      const auto rewardType = completed
        ? static_cast<protocol::QuestRewardType>(group.rewardType())
        : protocol::QuestRewardType::None;
      const uint32_t carrotsReward =
        rewardType == protocol::QuestRewardType::Carrots
          ? questDef->rewardGameMoney
          : 0;
      const uint32_t mountExp =
        rewardType == protocol::QuestRewardType::Exp
          ? questDef->rewardExp
          : 0;

      if (carrotsReward > 0)
      {
        characterRecord.Mutable([carrotsReward](data::Character& character)
        {
          character.carrots() += carrotsReward;
        });
      }

      if (mountExp > 0)
      {
        data::Uid mountUid = data::InvalidUid;
        characterRecord.Immutable([&mountUid](const data::Character& character)
        {
          mountUid = character.mountUid();
        });

        auto mountRecord = dataDirector.GetHorse(mountUid);
        if (mountRecord)
        {
          mountRecord.Mutable([this, mountExp](data::Horse& horse)
          {
            _serverInstance.GetHorseRegistry().ApplyClassProgress(horse, mountExp);
          });
        }
      }

      notifies.push_back({
        .characterUid = static_cast<uint32_t>(characterUid),
        .questId = static_cast<uint16_t>(entry.questId),
        .objectiveProgress = {
          .isCompleted = completed,
          .progress = entry.progress,
        },
        .carrotsReward = carrotsReward,
        .rewardType = rewardType,
        .unk2 = 0,
        .mountExp = mountExp,
      });
    }

    // Mark quests field as modified so it gets persisted
    group.quests = questSlots;
  });

  return notifies;
}

std::optional<QuestSystem::QuestEvent> QuestSystem::ToQuestEvent(const GameEvent::Kind kind)
{
  switch (kind)
  {
    case GameEvent::Kind::Any:
      return QuestEvent::Any;
    case GameEvent::Kind::PrizeWinner:
      return QuestEvent::PrizeWinner;
    case GameEvent::Kind::PerfectJump:
      return QuestEvent::PerfectJump;
    case GameEvent::Kind::FireballAttack:
      return QuestEvent::FireballAttack;
    case GameEvent::Kind::RunMap:
      return QuestEvent::RunMap;
    case GameEvent::Kind::TeamWin:
      return QuestEvent::TeamWin;
    case GameEvent::Kind::GlidingDistance:
      return QuestEvent::GlidingDistance;
    case GameEvent::Kind::CollectDropItem:
      return QuestEvent::CollectDropItem;
    case GameEvent::Kind::BoostUsed:
      return QuestEvent::BoostUsed;
    case GameEvent::Kind::FeedHorse:
      return QuestEvent::FeedHorse;
    case GameEvent::Kind::WashHorse:
      return QuestEvent::WashHorse;
    default:
      return std::nullopt;
  }
}

void QuestSystem::HandleGameEvent(const GameEvent& event)
{
  const auto questEvent = ToQuestEvent(event.kind);
  if (!questEvent)
    return;

  const auto notifies = OnQuestEvent(
    event.characterUid,
    *questEvent,
    event.gameMode,
    event.value);

  // Send the notify packets to the appropriate director based on the event origin
  switch (event.origin)
  {
    case GameEvent::Origin::Ranch:
    {
      auto& ranchDirector = _serverInstance.GetRanchDirector();
      for (const auto& notify : notifies)
        ranchDirector.SendDailyQuestNotificationToCharacter(event.characterUid, notify);
      break;
    }
    case GameEvent::Origin::Race:
    {
      auto& raceDirector = _serverInstance.GetRaceDirector();
      for (const auto& notify : notifies)
      {
        raceDirector.SendDailyQuestNotificationToCharacter(
          event.characterUid,
          notify.questId,
          notify.objectiveProgress,
          notify.carrotsReward,
          notify.rewardType,
          notify.mountExp);
      }
      break;
    }
  }
}

} // namespace server
