#include "server/race/mode/gamemode/SpeedModeHandler.hpp"
#include "server/ServerInstance.hpp"

namespace server::race::mode
{

void SpeedGameMode::OnHurdleClear(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] const protocol::AcCmdCRHurdleClearResult& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void SpeedGameMode::OnRaceUserPos(
  ClientId,
  RaceDirector::RaceInstance&,
  const protocol::AcCmdUserRaceUpdatePos&)
{
  // Base handler handles item spawning, do any speed related functions here
}

void SpeedGameMode::OnItemGet(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] const protocol::AcCmdUserRaceItemGet& command,
  [[maybe_unused]] tracker::RaceTracker::Item& item)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void SpeedGameMode::OnRequestSpur(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdCRRequestSpur& command)
{
  const auto& clientContext = _director.GetClientContext(clientId);

  std::scoped_lock lock(_director._raceInstancesMutex);

  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = _director.GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(raceInstance.raceGameMode));

  if (racer.starPointValue < gameModeTemplate.spurConsumeStarPoints)
    throw std::runtime_error("Client is dead ass cheating (or is really desynced)");

  racer.starPointValue -= gameModeTemplate.spurConsumeStarPoints;

  protocol::AcCmdCRRequestSpurOK response{
    .characterOid = command.characterOid,
    .activeBoosters = command.activeBoosters,
    .startPointValue = racer.starPointValue,
    .comboBreak = command.comboBreak};

  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  _director._commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  _director._commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]()
    {
      return starPointResponse;
    });
}

void SpeedGameMode::OnStartingRate(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] const protocol::AcCmdCRStartingRate& command)
{
  // TODO: copy implementation from RaceDirector
  throw std::logic_error("Not implemented");
}

void SpeedGameMode::OnUseMagicItem(
  ClientId,
  RaceDirector::RaceInstance&,
  const protocol::AcCmdCRUseMagicItem&)
{
  // Ignore in speed mode
}

} // namespace server::race::mode
