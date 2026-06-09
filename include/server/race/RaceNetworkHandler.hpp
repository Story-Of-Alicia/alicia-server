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

#ifndef ALICIA_SERVER_RACENETWORKHANDLER_HPP
#define ALICIA_SERVER_RACENETWORKHANDLER_HPP

#include "P2dIdPool.hpp"
#include "RaceInstance.hpp"

#include "server/Config.hpp"

#include "server/system/RoomSystem.hpp"
#include "server/tracker/RaceTracker.hpp"

#include "libserver/registry/MagicRegistry.hpp"
#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/CommonMessageDefinitions.hpp"
#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"
#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"
#include "libserver/util/Scheduler.hpp"

#include <random>
#include <unordered_map>

namespace server
{

class ServerInstance;

class RaceNetworkHandler final
  : public CommandServer::EventHandlerInterface
{
public:
  //!
  explicit RaceNetworkHandler(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

  void NotifyRoomNameChanged(uint32_t roomUid) noexcept;

  //! Send a RequestUser notification to a character connected to this director.
  void NotifySummonCharacter(
    data::Uid characterUid,
    bool force,
    std::string characterName,
    uint32_t roomUid,
    uint32_t ranchUid) noexcept;

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId clientId) override;

  void DisconnectCharacter(data::Uid characterUid);

  //! Get room count.
  //! @return Room count.
  [[nodiscard]] size_t GetRoomCount();

  Config::Race& GetConfig();

  ServerInstance& GetServerInstance();
  CommandServer& GetCommandServer();

  template <WritableStruct C>
  void Broadcast(
    const RaceInstance& raceInstance,
    const C& command)
  {
    raceInstance.GetRoom(
      [this, command](const Room& room)
      {
        for (const auto& player : room.GetPlayers() | std::views::values)
          _commandServer.QueueCommand<C>(
            player.GetClientId(),
            [command]()
            {
              return command;
            });
      });
  }

  template <WritableStruct C>
  void BroadcastExceptCharacterUid(
    const RaceInstance& raceInstance,
    const C& command,
    data::Uid skipCharacterUid)
  {
    raceInstance.GetRoom(
      [this, command, skipCharacterUid](const Room& room)
      {
        for (const auto& [characterUid, player] : room.GetPlayers())
        {
          if (characterUid == skipCharacterUid)
            continue;

          _commandServer.QueueCommand<C>(
            player.GetClientId(),
            [command]()
            {
              return command;
            });
        }
      });
  }

private:
  enum class EffectVerdict : uint8_t
  {
    Shielded,
    Applied,
    Duplicated,
    Failed
  };

  struct ClientContext
  {
    data::Uid characterUid{data::InvalidUid};
    data::Uid roomUid{data::InvalidUid};
    bool isAuthenticated = false;
    std::string userName;
  };

  race::P2dId GetOrCreateP2dId(ClientId clientId);

  ClientContext& GetClientContext(
    ClientId clientId,
    bool requireAuthorized = true);
  ClientId GetClientIdByCharacterUid(data::Uid characterUid);
  ClientContext& GetClientContextByCharacterUid(data::Uid characterUid);

  RaceInstance& GetRaceInstance(
    const ClientContext& clientContext,
    bool checkRacer = true);

  EffectVerdict ScheduleSkillEffect(
    RaceInstance& raceInstance,
    tracker::Oid attackerId, tracker::Oid targetId,
    const registry::Magic::SlotInfo& magicSlotInfo,
    uint16_t effectInstanceId = 0);

  //! Computes an effect's effective duration in milliseconds, applying any
  //! caster-stat duration bonus and any target-stat duration reduction from
  //! the spell's stat scaling (see magic.yaml statScalings).
  uint32_t ComputeEffectDurationMs(
    const registry::Magic::SlotInfo& magicSlotInfo,
    tracker::Oid attackerOid,
    const tracker::RaceTracker::Racer& targetRacer,
    const tracker::RaceTracker::RacerObjectMap& racers) const;

  void RemoveEffect(
    RaceInstance& raceInstance,
    tracker::RaceTracker::Racer& racer,
    uint32_t effectId);

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

  void SendStartRaceCancel(
    ClientId clientId,
    protocol::AcCmdCRStartRaceCancel::Reason reason);

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

  void HandleChangeMagicTarget(
    ClientId clientId,
    const protocol::AcCmdCRChangeMagicTarget& command);

  void HandleChangeSkillCardPresetId(
    ClientId clientId,
    const protocol::AcCmdCRChangeSkillCardPresetID& command);

  // Note: HandleActivateSkillEffect commented out due to build issues
  void HandleActivateSkillEffect(
    ClientId clientId,
    const protocol::AcCmdCRActivateSkillEffect& command);

  void HandleOpCmd(
    ClientId clientId,
    const protocol::AcCmdCROpCmd& command);

  //! Race clients can invite characters from ranch or other race rooms.
  void HandleInviteUser(
    ClientId clientId,
    const protocol::AcCmdCRInviteUser& command);

  void HandleRequestUser(
    ClientId clientId,
    const protocol::AcCmdCRRequestUser& command);

  void HandleKickUser(
    ClientId clientId,
    const protocol::AcCmdCRKick& command);

  //! Handles the team gauges in team races only.
  void HandleTeamGauge(ClientId clientId);

  void HandleTriggerizeAct(
    ClientId clientId,
    const protocol::AcCmdCRTriggerizeAct& command);

  void HandleGameCreateClientItem(
    ClientId clientId,
    const protocol::AcCmdCRGameCreateClientItem& command);

  //! A scheduler instance.
  Scheduler _scheduler;
  //! A server instance.
  ServerInstance& _serverInstance;
  //! A command server instance.
  CommandServer _commandServer;
  //! A map of all client contexts.
  std::unordered_map<ClientId, ClientContext> _clients;
  //! A map of all p2ds for UDP relay.
  std::unordered_map<ClientId, race::P2dId> _p2dIds;
  //! A pool for active race clients with P2dIds.
  race::P2dIdPool _p2dIdPool;

  std::mutex _raceInstancesMutex;
  //! A map of all race instanced indexed by room UIDs.
  std::unordered_map<uint32_t, RaceInstance> _raceInstances;
};

} // namespace server

#endif // ALICIA_SERVER_RACENETWORKHANDLER_HPP
