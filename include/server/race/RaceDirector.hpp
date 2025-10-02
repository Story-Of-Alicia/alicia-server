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

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId clientId) override;

  ServerInstance& GetServerInstance();
  Config::Race& GetConfig();

private:
  struct ClientContext
  {
    data::Uid characterUid{data::InvalidUid};
    data::Uid roomUid{data::InvalidUid};
    bool authorized = false;
  };
  ;

  struct RoomInstance
  {
    std::unordered_set<ClientId> clients;

    tracker::RaceTracker tracker;

    //! A leader character's UID.
    data::Uid masterUid{data::InvalidUid};
    
    //! Countdown start time for race timing
    std::optional<std::chrono::steady_clock::time_point> countdownStartTime;
    
    //! Actual race start timestamp (when countdown reaches 0)
    std::optional<uint64_t> raceStartTimestamp;
  };

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

  // Note: HandleActivateSkillEffect commented out due to build issues
  // void HandleActivateSkillEffect(
  //   ClientId clientId,
  //   const protocol::AcCmdCRActivateSkillEffect& command);

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
  std::unordered_map<uint32_t, RoomInstance> _roomInstances;
};

} // namespace server

#endif // RACEDIRECTOR_HPP
