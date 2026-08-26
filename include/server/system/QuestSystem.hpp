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

#ifndef QUESTSYSTEM_HPP
#define QUESTSYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/command/proto/CommonMessageDefinitions.hpp>
#include <libserver/network/command/proto/CommonStructureDefinitions.hpp>
#include <libserver/registry/QuestRegistry.hpp>

#include <vector>

namespace server
{

class ServerInstance;

class QuestSystem
{
public:
  explicit QuestSystem(ServerInstance& serverInstance);

  //! Events that can advance daily quest objectives.
  //! Each value corresponds to a quest `function` string in quests.yaml.
  enum class QuestEvent
  {
    //! Any action (used by quests with function 'TRUE').
    Any,
    //! Finished in a placing position (1st–3rd).
    PrizeWinner,
    //! Achieved a perfect jump over a hurdle.
    PerfectJump,
    //! Used a fireball / magic attack.
    FireballAttack,
    //! Completed a specific map (value = map block ID).
    RunMap,
    //! Won a team race.
    TeamWin,
    //! Accumulated gliding distance (value = distance units).
    GlidingDistance,
    //! Collected a drop item during a race.
    CollectDropItem,
  };

  //! Evaluates all active daily quests for a character against the given event
  //! and advances progress on any matching quests.
  //! @param characterUid UID of the character.
  //! @param event The event that occurred.
  //! @param gameMode The game mode in which the event occurred.
  //! @param value Optional scalar value for the event (e.g. map ID, distance).
  //! @returns A list of notify packets to be sent to the character by the caller.
  [[nodiscard]] std::vector<protocol::AcCmdRCUpdateDailyQuestNotify> OnQuestEvent(
    data::Uid characterUid,
    QuestEvent event,
    registry::Quest::GameModeFlag gameMode,
    uint32_t value = 0);

  //! Converts a protocol GameMode + TeamMode pair to the matching GameModeFlag
  //! used by the quest registry for mode-based filtering.
  //! @param gameMode Speed or Magic.
  //! @param teamMode Team or Solo.
  //! @returns The corresponding GameModeFlag value.
  static registry::Quest::GameModeFlag ToGameModeFlag(
    protocol::GameMode gameMode,
    protocol::TeamMode teamMode);

private:
  //! Returns true if the quest's gameModeFlag is compatible with the given mode.
  static bool IsModeMatch(
    registry::Quest::GameModeFlag questFlag,
    registry::Quest::GameModeFlag eventMode);

  //! Returns true if the quest's function matches the given event.
  static bool IsEventMatch(
    registry::Quest::Function function,
    QuestEvent event,
    uint32_t questFunctionValue,
    uint32_t eventValue);

  ServerInstance& _serverInstance;
};

} // namespace server

#endif // QUESTSYSTEM_HPP
