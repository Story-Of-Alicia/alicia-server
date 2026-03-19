#include "server/race/mode/gamemode/TutorialModeHandler.hpp"

namespace server::race::mode
{

void TutorialGameMode::OnHurdleClear(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRHurdleClearResult& command)
{
  // TODO: complete implementation with a MissionRegistry, identify speed or magic
  // based on missionId from libconfig
  throw std::logic_error("Not implemented");
}

void TutorialGameMode::OnRaceUserPos(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdUserRaceUpdatePos& command)
{
  // TODO: complete implementation with a MissionRegistry, identify speed or magic
  // based on missionId from libconfig
  throw std::logic_error("Not implemented");
}

void TutorialGameMode::OnItemGet(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdUserRaceItemGet& command,
  [[maybe_unused]] tracker::RaceTracker::Item& item)
{
  // TODO: complete implementation with a MissionRegistry, identify speed or magic
  // based on missionId from libconfig
  throw std::logic_error("Not implemented");
}

void TutorialGameMode::OnRequestSpur(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRRequestSpur& command)
{
  // TODO: complete implementation with a MissionRegistry, identify speed or magic
  // based on missionId from libconfig
  throw std::logic_error("Not implemented");
}

void TutorialGameMode::OnStartingRate(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRStartingRate& command)
{
  // TODO: complete implementation with a MissionRegistry, identify speed or magic
  // based on missionId from libconfig
  throw std::logic_error("Not implemented");
}

void TutorialGameMode::OnUseMagicItem(
  [[maybe_unused]] RaceDirector& director,
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer,
  [[maybe_unused]] const protocol::AcCmdCRUseMagicItem& command)
{
  // TODO: complete implementation with a MissionRegistry, identify speed or magic
  // based on missionId from libconfig
  throw std::logic_error("Not implemented");
}

} // namespace server::race::mode