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
  const registry::GameModeFlag questFlag,
  const registry::GameModeFlag eventMode)
{
  // Flag None (0): no mode restriction
  if (questFlag == registry::GameModeFlag::None)
    return true;
  // Flag Any (111): explicitly matches all race modes
  if (questFlag == registry::GameModeFlag::Any)
    return true;
  return questFlag == eventMode;
}

bool QuestSystem::Matches(
  const registry::Quest& quest,
  const GameEvent& event)
{
  if (quest.userAchvEvent != static_cast<uint32_t>(event.userAchvEvent))
    return false;

  if (quest.function != event.function)
    return false;

  if (not IsModeMatch(quest.gameModeFlag, event.gameMode))
    return false;

  return quest.functionValue == 0 || quest.functionValue == event.value;
}

std::vector<protocol::AcCmdRCUpdateDailyQuestNotify> QuestSystem::OnQuestEvent(
  const data::Uid characterUid,
  const GameEvent& event)
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

      if (not Matches(*questDef, event))
      {
        spdlog::debug(
          "QuestSystem::OnQuestEvent: quest {} does not match event "
          "(quest uae {} fn {} fnValue {} mode {}; event uae {} fn {} value {} mode {})",
          entry.questId,
          questDef->userAchvEvent,
          static_cast<uint32_t>(questDef->function),
          questDef->functionValue,
          static_cast<uint32_t>(questDef->gameModeFlag),
          static_cast<uint32_t>(event.userAchvEvent),
          static_cast<uint32_t>(event.function),
          event.value,
          static_cast<uint32_t>(event.gameMode));
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

      uint32_t carrotsTotal = 0;

      if (carrotsReward > 0)
      {
        characterRecord.Mutable([carrotsReward, &carrotsTotal](data::Character& character)
        {
          character.carrots() += carrotsReward;
          carrotsTotal = static_cast<uint32_t>(character.carrots());
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
        .carrotsReward = carrotsTotal,
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

void QuestSystem::HandleGameEvent(const GameEvent& event)
{
  if (event.userAchvEvent == registry::UserAchvEvent::None)
    return;

  const auto notifies = OnQuestEvent(event.characterUid, event);

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
