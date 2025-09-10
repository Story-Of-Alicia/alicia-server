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

#include "server/system/InfractionSystem.hpp"

#include "server/ServerInstance.hpp"

namespace server
{

InfractionSystem::InfractionSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

InfractionSystem::Verdict InfractionSystem::CheckOutstandingPunishments(const std::string& userName)
{
  const auto userRecord = _serverInstance.GetDataDirector().GetUser(userName);
  if (not userRecord)
    throw std::runtime_error("Couldn't check outstanding infractions, user not available");

  Verdict verdict;
  userRecord.Immutable([this, &verdict](const data::User& user)
  {
    const auto infractionRecords = _serverInstance.GetDataDirector().GetInfractionCache().Get(
      user.infractions());

    for (const auto& infractionRecord : *infractionRecords)
    {
      infractionRecord.Immutable([&verdict](const data::Infraction& infraction)
      {
        const bool expired = infraction.createdAt() + infraction.duration() < data::Clock::now();
        if (expired)
          return;

        switch (infraction.punishment())
        {
          case data::Infraction::Punishment::Mute:
            verdict.preventChatting = true;
            break;
          case data::Infraction::Punishment::Ban:
            verdict.preventServerJoining = true;
            break;
          default:
            break;
        }
      });
    }
  });

  return verdict;
}

} // namespace server