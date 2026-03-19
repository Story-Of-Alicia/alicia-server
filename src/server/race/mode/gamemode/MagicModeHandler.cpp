#include "server/race/mode/gamemode/MagicModeHandler.hpp"

namespace server::race::mode
{

void MagicGameMode::OnHurdleClear(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRHurdleClearResult& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnRaceUserPos(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdUserRaceUpdatePos& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnItemGet(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdUserRaceItemGet& command,
  [[maybe_unused]] tracker::RaceTracker::Item& item)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnRequestSpur(
  RaceDirector&,
  ClientId,
  RaceDirector::RaceInstance&,
  tracker::RaceTracker::Racer&,
  const protocol::AcCmdCRRequestSpur&)
{
  // Ignore request spur (speed) in magic mode
}

void MagicGameMode::OnStartingRate(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRStartingRate& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void MagicGameMode::OnUseMagicItem(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRUseMagicItem& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

} // namespace server::race::mode
