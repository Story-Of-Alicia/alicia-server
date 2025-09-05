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

#ifndef RANCHDIRECTOR_HPP
#define RANCHDIRECTOR_HPP

#include "server/Config.hpp"
#include "server/tracker/WorldTracker.hpp"

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"

#include <random>
#include <unordered_map>
#include <unordered_set>

namespace server
{

class ServerInstance;

class RanchDirector final
  : public CommandServer::EventHandlerInterface
{
public:
  //!
  explicit RanchDirector(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

  std::vector<data::Uid> GetOnlineCharacters();

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId client) override;

  //!
  void BroadcastSetIntroductionNotify(
    uint32_t characterUid,
    const std::string& introduction);

  //!
  void BroadcastUpdateMountInfoNotify(
    data::Uid characterUid,
    data::Uid rancherUid,
    data::Uid horseUid);

  //! Show popup notification for client indicating a new item in storage, by character UID
  void SendStorageNotification(
    data::Uid characterUid,
    protocol::AcCmdCRRequestStorage::Category category);

  ServerInstance& GetServerInstance();
  Config::Ranch& GetConfig();

private:
  std::random_device _randomDevice;

  struct ClientContext
  {
    bool isAuthorized{false};
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Unique ID of the owner of the ranch the client is visiting.
    data::Uid visitingRancherUid{data::InvalidUid};

    
    uint8_t busyState{0};
  };

  struct RanchInstance
  {
    //! A world tracker of the ranch.
    WorldTracker worldTracker;
    //! A set of clients connected to the ranch.
    std::unordered_set<ClientId> clients;
  };

  //! Get client context.
  //! @param clientId Id of the client.
  //! @param requireAuthorized Require the client to be authorized.
  //! @returns Client context.
  [[nodiscard]] ClientContext& GetClientContext(ClientId clientId, bool requireAuthorized = true);

  //! Get the client context by the character's unique ID.
  //! @param characterUid UID of the character.
  //! @returns Client context.
  [[nodiscard]] ClientContext& GetClientContextByCharacterUid(data::Uid characterUid);

  //! Handles the ranch enter command.
  //! @param clientId ID of the client
  //! @param command Command
  void HandleEnterRanch(
    ClientId clientId,
    const protocol::AcCmdCREnterRanch& command);

  void HandleRanchLeave(
    ClientId clientId);

  void HandleChat(
    ClientId clientId,
    const protocol::AcCmdCRRanchChat& command);

  void HandleSnapshot(
    ClientId clientId,
    const protocol::AcCmdCRRanchSnapshot& command);

  void HandleEnterBreedingMarket(
    ClientId clientId,
    const protocol::AcCmdCREnterBreedingMarket& command);

  void HandleSearchStallion(
    ClientId clientId,
    const protocol::AcCmdCRSearchStallion& command);

  void HandleRegisterStallion(
    ClientId clientId,
    const protocol::AcCmdCRRegisterStallion& command);

  void HandleUnregisterStallion(
    ClientId clientId,
    const protocol::AcCmdCRUnregisterStallion& command);

  void HandleUnregisterStallionEstimateInfo(
    ClientId clientId,
    const protocol::AcCmdCRUnregisterStallionEstimateInfo& command);

  void HandleTryBreeding(
    ClientId clientId,
    const protocol::AcCmdCRTryBreeding& command);

  void HandleBreedingAbandon(
    ClientId clientId,
    const protocol::AcCmdCRBreedingAbandon& command);

  //!
  void HandleBreedingWishlist(
    ClientId clientId,
    const protocol::RanchCommandBreedingWishlist& command);

  //!
  void HandleCmdAction(
    ClientId clientId,
    const protocol::AcCmdCRRanchCmdAction& command);

  //!
  void HandleRanchStuff(
    ClientId clientId,
    const protocol::RanchCommandRanchStuff& command);

  //!
  void HandleUpdateBusyState(
    ClientId clientId,
    const protocol::RanchCommandUpdateBusyState& command);

  //!
  void HandleUpdateMountNickname(
    ClientId clientId,
    const protocol::RanchCommandUpdateMountNickname& command);

  //!
  void HandleRequestStorage(
    ClientId clientId,
    const protocol::AcCmdCRRequestStorage& command);

  //!
  void HandleGetItemFromStorage(
    ClientId clientId,
    const protocol::AcCmdCRGetItemFromStorage& command);

  //!
  void HandleRequestNpcDressList(
    ClientId clientId,
    const protocol::RanchCommandRequestNpcDressList& requestNpcDressList);

  void HandleWearEquipment(
    ClientId clientId,
    const protocol::AcCmdCRWearEquipment& command);

  void HandleRemoveEquipment(
    ClientId clientId,
    const protocol::AcCmdCRRemoveEquipment& command);

  void HandleCreateGuild(
    ClientId clientId,
    const protocol::RanchCommandCreateGuild& command);

  void HandleRequestGuildInfo(
    ClientId clientId,
    const protocol::RanchCommandRequestGuildInfo& command);

  void HandleLeaveGuild(
    ClientId clientId,
    const protocol::AcCmdCRWithdrawGuildMember& command);

  void HandleUpdatePet(
    ClientId clientId,
    const protocol::AcCmdCRUpdatePet& command);
  
  void HandleIncubateEgg(
    ClientId clientId,
    const protocol::AcCmdCRIncubateEgg& command);

  void HandleBoostIncubateInfoList(
    ClientId clientId,
    const protocol::AcCmdCRBoostIncubateInfoList& command);
  
  void HandleBoostIncubateEgg(
    ClientId clientId,
    const protocol::AcCmdCRBoostIncubateEgg& command);

  void HandleRequestPetBirth(
    ClientId clientId,
    const protocol::AcCmdCRRequestPetBirth& command);

  void HandleUserPetInfos(
    ClientId clientId,
    const protocol::RanchCommandUserPetInfos& command);

  //! Broadcasts an equipment update of the character owned by the client
  //! to the currently connected ranch.
  //! @param clientId ID of the client.
  void BroadcastEquipmentUpdate(
    ClientId clientId);

  void HandleUseFeedItem(
    const protocol::AcCmdCRUseItem& command,
    protocol::AcCmdCRUseItemOK& response);

  void HandleUseCleanItem(
    const protocol::AcCmdCRUseItem& command,
    protocol::AcCmdCRUseItemOK& response);
  
  void HandleUsePlayItem(
    const protocol::AcCmdCRUseItem& command,
    protocol::AcCmdCRUseItemOK& response);

  void HandleUseCureItem(
    const protocol::AcCmdCRUseItem& command,
    protocol::AcCmdCRUseItemOK& response);

  void HandleUseItem(
    ClientId clientId,
    const protocol::AcCmdCRUseItem& command);

  void HandleHousingBuild(
    ClientId clientId,
    const protocol::AcCmdCRHousingBuild& command);

  void HandleHousingRepair(
    ClientId clientId,
    const protocol::AcCmdCRHousingRepair& command);
  
  void HandleOpCmd(ClientId clientId,
    const protocol::RanchCommandOpCmd& command);

  void HandleRequestLeagueTeamList(ClientId clientId,
    const protocol::RanchCommandRequestLeagueTeamList& command);

  void HandleMountFamilyTree(ClientId clientId,
    const protocol::RanchCommandMountFamilyTree& command);

  void HandleRecoverMount(
    ClientId clientId,
    const protocol::AcCmdCRRecoverMount command);

  void HandleCheckStorageItem(
    ClientId clientId,
    const protocol::AcCmdCRCheckStorageItem command);

  //!
  ServerInstance& _serverInstance;
  //!
  CommandServer _commandServer;

  //!
  std::unordered_map<ClientId, ClientContext> _clients;
  //!
  std::unordered_map<data::Uid, RanchInstance> _ranches;
};

} // namespace server

#endif // RANCHDIRECTOR_HPP
