#include "server/race/mode/teammode/TeamRaceMode.hpp"

#include "server/system/RoomSystem.hpp"

namespace server::race::mode
{

bool TeamRaceMode::AreTeamsBalanced(server::Room& room) const
{
  uint32_t redTeamCount = 0;
  uint32_t blueTeamCount = 0;

  for (const auto& [characterUid, player] : room.GetPlayers())
  {
    switch (player.GetTeam())
    {
      case Room::Player::Team::Red:
        redTeamCount++;
        break;
      case Room::Player::Team::Blue:
        blueTeamCount++;
        break;
      default:
        break;
    }
  }

  return redTeamCount == blueTeamCount;
}

bool TeamRaceMode::IsEnemy(
  const tracker::RaceTracker::Racer& a,
  const tracker::RaceTracker::Racer& b) const
{
  return a.oid != b.oid && a.team != b.team;
}

bool TeamRaceMode::IsAlly(
  const tracker::RaceTracker::Racer& a,
  const tracker::RaceTracker::Racer& b) const
{
  return a.oid != b.oid && a.team == b.team;
}

void TeamRaceMode::OnTeamGauge(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer)
{
  // TODO: copy implementation from RaceDirector::HandleTeamGauge
}

} // namespace server::race::mode
