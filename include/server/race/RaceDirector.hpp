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

#ifndef RACEDIRECTOR_HPP
#define RACEDIRECTOR_HPP

#include "server/Config.hpp"

#include "server/tracker/RaceTracker.hpp"

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"
#include "libserver/util/Scheduler.hpp"

#include <random>
#include <unordered_map>
#include <unordered_set>

namespace server
{

class ServerInstance;

} // namespace server

namespace server
{

class RaceDirector final
  : public CommandServer::EventHandlerInterface
{
public:
  //!
  explicit RaceDirector(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

  bool IsRoomRacing(uint32_t uid)
  {
    const auto roomIter = _raceInstances.find(uid);
    if (roomIter == _raceInstances.cend())
      return false;

    return roomIter->second.stage == RoomInstance::Stage::Racing |
      roomIter->second.stage == RoomInstance::Stage::Loading;
  }

  uint32_t GetRoomPlayerCount(uint32_t uid)
  {
    const auto roomIter = _raceInstances.find(uid);
    if (roomIter == _raceInstances.cend())
      return 0;

    return roomIter->second.tracker.GetRacers().size();
  }

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId clientId) override;

  void DisconnectCharacter(data::Uid characterUid);

  ServerInstance& GetServerInstance();
  Config::Race& GetConfig();

private:
  std::random_device _randomDevice;

  struct ClientContext
  {
    data::Uid characterUid{data::InvalidUid};
    data::Uid roomUid{data::InvalidUid};
    bool isAuthenticated = false;
  };

  struct RoomInstance
  {
    //! A stage of the room.
    enum class Stage
    {
      Waiting,
      Loading,
      Racing,
      Finishing,
    } stage{Stage::Waiting};
    //! A time point of when the stage timeout occurs.
    std::chrono::steady_clock::time_point stageTimeoutTimePoint;

    //! A master's character UID.
    data::Uid masterUid{data::InvalidUid};
    //! A race object tracker.
    tracker::RaceTracker tracker;

    //! A game mode of the race.
    protocol::GameMode raceGameMode;
    //! A team mode of the race.
    protocol::TeamMode raceTeamMode;
    //! A map block ID of the race.
    uint16_t raceMapBlockId{};
    //! A mission ID of the race.
    uint16_t raceMissionId{};

    //! A time point of when the race is actually started (a countdown is finished).
    std::chrono::steady_clock::time_point raceStartTimePoint;
    //! A room clients.
    std::unordered_set<ClientId> clients;
  };

  ClientContext& GetClientContext(ClientId clientId, bool requireAuthorized = true);
  ClientId GetClientIdByCharacterUid(data::Uid characterUid);
  ClientContext& GetClientContextByCharacterUid(data::Uid characterUid);

  void HandleEnterRoom(
    ClientId clientId,
    const protocol::AcCmdCREnterRoom& command);

  void HandleChangeRoomOptions(
    ClientId clientId,
    const protocol::AcCmdCRChangeRoomOptions& command);

  void HandleChangeTeam(
    ClientId clientId,
    const protocol::AcCmdCRChangeTeam& command);

  void HandleLeaveRoom(
    ClientId clientId);

  void HandleReadyRace(
  ClientId clientId,
  const protocol::AcCmdCRReadyRace& command);

  void HandleStartRace(
    ClientId clientId,
    const protocol::AcCmdCRStartRace& command);

  void HandleRaceTimer(
    ClientId clientId,
    const protocol::AcCmdUserRaceTimer& command);

  void HandleLoadingComplete(
    ClientId clientId,
    const protocol::AcCmdCRLoadingComplete& command);

  void HandleUserRaceFinal(
    ClientId clientId,
    const protocol::AcCmdUserRaceFinal& command);

  void HandleRaceResult(
    ClientId clientId,
    const protocol::AcCmdCRRaceResult& command);

  void HandleP2PRaceResult(
    ClientId clientId,
    const protocol::AcCmdCRP2PResult& command);

  void HandleP2PUserRaceResult(
    ClientId clientId,
    const protocol::AcCmdUserRaceP2PResult& command);

  void HandleAwardStart(
    ClientId clientId,
    const protocol::AcCmdCRAwardStart& command);

  void HandleAwardEnd(
    ClientId clientId,
    const protocol::AcCmdCRAwardEnd& command);

  void HandleStarPointGet(
    ClientId clientId,
    const protocol::AcCmdCRStarPointGet& command);

  void HandleRequestSpur(
    ClientId clientId,
    const protocol::AcCmdCRRequestSpur& command);

  void HandleHurdleClearResult(
    ClientId clientId,
    const protocol::AcCmdCRHurdleClearResult& command);

  void HandleStartingRate(
    ClientId clientId,
    const protocol::AcCmdCRStartingRate& command);

  void HandleRaceUserPos(
    ClientId clientId,
    const protocol::AcCmdUserRaceUpdatePos& command);

  void HandleChat(
    ClientId clientId,
    const protocol::AcCmdCRChat& command);

  void HandleRelayCommand(
    ClientId clientId,
    const protocol::AcCmdCRRelayCommand& command);

  void HandleRelay(
    ClientId clientId,
    const protocol::AcCmdCRRelay& command);

  void HandleUserRaceActivateInteractiveEvent(
    ClientId clientId,
    const protocol::AcCmdUserRaceActivateInteractiveEvent& command);

  void HandleUserRaceActivateEvent(
    ClientId clientId,
    const protocol::AcCmdUserRaceActivateEvent& command);

  void HandleUserRaceDeactivateEvent(
    ClientId clientId,
    const protocol::AcCmdUserRaceDeactivateEvent& command);

  void HandleRequestMagicItem(
    ClientId clientId,
    const protocol::AcCmdCRRequestMagicItem& command);

  void HandleUseMagicItem(
    ClientId clientId,
    const protocol::AcCmdCRUseMagicItem& command);

  void HandleUserRaceItemGet(
    ClientId clientId,
    const protocol::AcCmdUserRaceItemGet& command);

  // Magic Targeting Commands for Bolt System
  void HandleStartMagicTarget(
    ClientId clientId,
    const protocol::AcCmdCRStartMagicTarget& command);

  void HandleChangeMagicTargetNotify(
    ClientId clientId,
    const protocol::AcCmdCRChangeMagicTargetNotify& command);

  void HandleChangeMagicTargetOK(
    ClientId clientId,
    const protocol::AcCmdCRChangeMagicTargetOK& command);

  void HandleChangeMagicTargetCancel(
    ClientId clientId,
    const protocol::AcCmdCRChangeMagicTargetCancel& command);

  void HandleChangeSkillCardPresetId(
    ClientId clientId,
    const protocol::AcCmdCRChangeSkillCardPresetID& command);

  // Note: HandleActivateSkillEffect commented out due to build issues
  void HandleActivateSkillEffect(
    ClientId clientId,
    const protocol::AcCmdCRActivateSkillEffect& command);

  void PrepareItemSpawners(data::Uid roomUid);

  //!
  std::thread test;
  std::atomic_bool run_test{true};

  //! A scheduler instance.
  Scheduler _scheduler;
  //! A server instance.
  ServerInstance& _serverInstance;
  //! A command server instance.
  CommandServer _commandServer;
  //! A map of all client contexts.
  std::unordered_map<ClientId, ClientContext> _clients;
  //! A map of all room instances.
  std::unordered_map<uint32_t, RoomInstance> _raceInstances;
};

} // namespace server

#endif // RACEDIRECTOR_HPP
