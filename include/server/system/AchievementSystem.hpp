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

#ifndef ACHIEVEMENTSYSTEM_HPP
#define ACHIEVEMENTSYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/command/proto/RaceMessageDefinitions.hpp>
#include <libserver/registry/AchievementRegistry.hpp>

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace server
{

class ServerInstance;

//! System responsible for achievement progress and for the derived state of the
//! achievement page. The client reports over whichever connection it holds, the
//! ranch one or the race one, so both handlers feed into this one place.
class AchievementSystem
{
public:
  //! What a reported property did to a single achievement, so the caller can
  //! notify the client over its own connection.
  struct ProgressUpdate
  {
    //! Achievement the update belongs to.
    uint16_t achievementTid{};
    //! Progress after the update.
    uint32_t progress{};
    //! Whether this update reached a new tier.
    bool reachedTier{};
    //! Tier that was reached, only meaningful when one was.
    uint8_t tier{};
    //! Carrot balance after any reward was paid.
    int32_t carrotBalance{};
  };

  //! What a finished race amounted to for one racer. The client reports none of
  //! this, so the server describes the race itself.
  struct RaceOutcome
  {
    //! Mode the race ran in, which decides whether an achievement counts.
    registry::GameModeFlag mode{};
    //! Count of racers, for the achievements that demand a minimum field.
    uint8_t racerCount{};
    //! Place the racer took, one for the winner. Zero when they gave up.
    uint8_t placement{};
    //! Whether the racer reached the goal rather than giving up.
    bool finished{};
    //! Hour of the local day the race ended in, for the time of day conditions.
    uint8_t hourOfDay{};
  };

  explicit AchievementSystem(ServerInstance& serverInstance);

  //! Turns an update into the notification the client expects, which both
  //! handlers send over their own connection.
  //! @param update What changed for one achievement.
  //! @returns The notification to queue.
  [[nodiscard]] static protocol::AcCmdRCAchievementUpdateNotify BuildUpdateNotify(
    const ProgressUpdate& update);

  //! Applies a finished race to the achievements that wait on the result.
  //! @param characterUid The racer.
  //! @param outcome What the race amounted to for them.
  //! @returns One entry per achievement whose progress changed.
  [[nodiscard]] std::vector<ProgressUpdate> ApplyRaceOutcome(
    data::Uid characterUid,
    const RaceOutcome& outcome);

  //! Applies a property value the client reported for an event.
  //! @param characterUid The character the report belongs to.
  //! @param eventId The event the client reported.
  //! @param propertyValue The reported value as the client sent it.
  //! @returns One entry per achievement whose progress changed.
  [[nodiscard]] std::vector<ProgressUpdate> ApplyReportedProperty(
    data::Uid characterUid,
    uint16_t eventId,
    const std::string& propertyValue);

  //! Ends the play session, breaking the runs that have to hold within one
  //! login.
  //! @param characterUid The character that left the game.
  void ApplySessionEnd(data::Uid characterUid);

  //! Forgets what a character reported, so the next report counts in full. Done
  //! when a race starts and on logout.
  //! @param characterUid The character that left or entered a race.
  void ForgetReportedValues(data::Uid characterUid);

  //! Count of tiers carrying a date and therefore earned, filled in order.
  //! @param entry The achievement record.
  //! @returns The count of earned tiers.
  [[nodiscard]] static uint8_t GetEarnedTierCount(
    const data::Character::AchievementEntry& entry);

  //! Count of tiers every achievement of a book has reached.
  //! @param character The character.
  //! @param bookType The book type.
  //! @returns A count in an interval <0, 4>, zero for an empty book.
  [[nodiscard]] uint8_t GetBookCompletedTierCount(
    const data::Character& character,
    int8_t bookType) const;

  //! Grade to report for a book, the index of the highest tier every
  //! achievement of the book has reached.
  //! @param character The character.
  //! @param bookType The book type.
  //! @returns A grade in an interval <0, 3>, bronze while none is earned.
  [[nodiscard]] uint8_t GetBookGrade(
    const data::Character& character,
    int8_t bookType) const;

private:
  ServerInstance& _serverInstance;

  //! Guards the reported values, which two network handlers reach from their
  //! own threads.
  std::mutex _reportedValuesMutex;
  //! Last value each character reported per event, from which the difference to
  //! the next one is the new progress. Outlives a single connection, which is
  //! why it does not live in a client context.
  std::unordered_map<data::Uid, std::unordered_map<uint16_t, uint32_t>>
    _reportedValues;
};

} // namespace server

#endif // ACHIEVEMENTSYSTEM_HPP
