#include "server/race/mode/teammode/FfaTeamMode.hpp"

namespace server::race::mode
{

bool FfaTeamMode::AreTeamsBalanced(server::Room&) const
{
  return true; // Always balanced in FFA
}

bool FfaTeamMode::IsEnemy(
  const tracker::RaceTracker::Racer& a,
  const tracker::RaceTracker::Racer& b) const
{
  return a.oid != b.oid;
}

bool FfaTeamMode::IsAlly(
  const tracker::RaceTracker::Racer&,
  const tracker::RaceTracker::Racer&) const
{
  return false;
}

void FfaTeamMode::OnTeamGauge(
  ClientId,
  RaceDirector::RaceInstance&,
  tracker::RaceTracker::Racer&)
{
  // No team gauge in FFA
}

} // namespace server::race::mode
