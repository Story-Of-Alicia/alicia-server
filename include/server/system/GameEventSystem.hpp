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

#ifndef GAMEEVENTSYSTEM_HPP
#define GAMEEVENTSYSTEM_HPP

#include "server/event/GameEvent.hpp"

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/command/proto/CommonStructureDefinitions.hpp>
#include <libserver/registry/QuestRegistry.hpp>

#include <string_view>

namespace server
{

class ServerInstance;

//! Translates raw occurrences (a client-reported achievement property, a
//! race mode pairing, ...) into `GameEvent`s and fires them on the shared
//! game event bus. Home for logic shared by every listener on that bus
//! (quests today; achievements and player statistics are expected to
//! subscribe the same way later), as opposed to logic specific to one of
//! them.
class GameEventSystem
{
public:
  explicit GameEventSystem(ServerInstance& serverInstance);

  GameEventSystem(const GameEventSystem&) = delete;
  GameEventSystem& operator=(GameEventSystem&) = delete;
  GameEventSystem(GameEventSystem&&) = delete;
  GameEventSystem& operator=(GameEventSystem&&) = delete;

  //! Fires a `GameEvent` for a client-reported achievement property.
  //! @param characterUid Character the report is about.
  //! @param origin Where the report came from.
  //! @param achievementEvent Reported category, as sent by the client.
  //! @param achievementValue Textual value the client sent alongside it.
  //! @param gameMode Game mode the report occurred in, if applicable.
  void ReportAchievement(
    data::Uid characterUid,
    GameEvent::Origin origin,
    registry::UserAchvEvent achievementEvent,
    std::string_view achievementValue,
    registry::Quest::GameModeFlag gameMode = registry::Quest::GameModeFlag::None);

  //! Converts a protocol GameMode + TeamMode pair to the matching GameModeFlag
  //! @param gameMode Speed or Magic.
  //! @param teamMode Team or Solo.
  //! @returns The corresponding GameModeFlag value.
  [[nodiscard]] static registry::Quest::GameModeFlag ToGameModeFlag(
    protocol::GameMode gameMode,
    protocol::TeamMode teamMode);

  //! Simmilar to "ToGameModeFlag", but returns the flag for winning the race
  //! rather than merely completing it.
  //! @param gameMode Speed or Magic.
  //! @param teamMode Team or Solo.
  //! @returns The corresponding GameModeFlag value.
  [[nodiscard]] static registry::Quest::GameModeFlag ToWinGameModeFlag(
    protocol::GameMode gameMode,
    protocol::TeamMode teamMode);

private:
  //! Whether the server should ignore a client-reported achievement property.
  //! @param achievementEvent Reported category.
  //! @returns True when the report should be ignored.
  [[nodiscard]] static bool IsReportedNatively(
    registry::UserAchvEvent achievementEvent);

  //! Maps a client-reported achievement property to the corresponding quest function.
  //! @param achievementEvent Reported category.
  //! @returns The matching function.
  [[nodiscard]] static registry::Function ToFunction(
    registry::UserAchvEvent achievementEvent);

  //! Parses the textual value the client sends with an achievement property.
  //! Counters arrive as plain integers, timings and distances as "%.2f".
  //! @param value Textual value.
  //! @returns The value truncated to an integer, or 0 when unparsable.
  [[nodiscard]] static uint32_t ParseAchievementPropertyValue(std::string_view value);

  ServerInstance& _serverInstance;
};

} // namespace server

#endif // GAMEEVENTSYSTEM_HPP
