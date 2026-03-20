#include "server/race/mode/gamemode/SpeedModeHandler.hpp"

namespace server::race::mode
{

void SpeedGameMode::OnHurdleClear(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRHurdleClearResult& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void SpeedGameMode::OnRaceUserPos(
  ClientId,
  RaceDirector::RaceInstance&,
  tracker::RaceTracker::Racer&,
  const protocol::AcCmdUserRaceUpdatePos&)
{
  // Base handler handles item spawning, do any speed related functions here
}

void SpeedGameMode::OnItemGet(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdUserRaceItemGet& command,
  [[maybe_unused]] tracker::RaceTracker::Item& item)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void SpeedGameMode::OnRequestSpur(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRRequestSpur& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void SpeedGameMode::OnStartingRate(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRStartingRate& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void SpeedGameMode::OnUseMagicItem(
  ClientId,
  RaceDirector::RaceInstance&,
  tracker::RaceTracker::Racer&,
  const protocol::AcCmdCRUseMagicItem&)
{
  // Ignore in speed mode
}

} // namespace server::race::mode
