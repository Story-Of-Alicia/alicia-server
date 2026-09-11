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

#include "server/event/GameEvent.hpp"

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

  //! Evaluates the character's active daily quests against an event and
  //! advances any that it satisfies.
  //! @param characterUid UID of the character.
  //! @param event The event that occurred.
  //! @returns Notify packets for the caller to send to the character.
  [[nodiscard]] std::vector<protocol::AcCmdRCUpdateDailyQuestNotify> OnQuestEvent(
    data::Uid characterUid,
    const GameEvent& event);

  //! Tests whether an event satisfies a quest's completion condition.
  //! @param quest Quest definition.
  //! @param event The event that occurred.
  //! @returns True when the event advances the quest.
  [[nodiscard]] static bool Matches(
    const registry::Quest& quest,
    const GameEvent& event);

private:
  //! Returns true if the quest's gameModeFlag is compatible with the given mode.
  static bool IsModeMatch(
    registry::Quest::GameModeFlag questFlag,
    registry::Quest::GameModeFlag eventMode);

  //! Listener for the server's game event bus.
  void HandleGameEvent(const GameEvent& event);

  ServerInstance& _serverInstance;
  //! Handle for this system's subscription to the game event bus.
  GameEventBus::ListenerHandle _gameEventListenerHandle;
};

} // namespace server

#endif // QUESTSYSTEM_HPP
