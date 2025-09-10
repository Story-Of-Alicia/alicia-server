//
// Created by rgnter on 10/09/2025.
//

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