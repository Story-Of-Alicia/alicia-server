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

#include "server/Settings.hpp"
#include "server/tracker/WorldTracker.hpp"

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"

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

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId client) override;

  //!
  void BroadcastSetIntroductionNotify(
    uint32_t characterUid,
    const std::string& introduction);

  //!
  void BroadcastUpdateMountInfoNotify(
    data::Uid characterUid,
    data::Uid horseUid);

  ServerInstance& GetServerInstance();
  Settings::RanchSettings& GetSettings();

private:
  struct ClientContext
  {
    data::Uid characterUid{data::InvalidUid};
    data::Uid rancherUid{data::InvalidUid};
    uint8_t busyState{0};
  };

  ClientContext& GetClientContextByCharacterUid(data::Uid characterUid);

  //!
  void HandleRanchEnter(
    ClientId clientId,
    const protocol::RanchCommandRanchEnter& command);

  //!
  void HandleRanchLeave(
    ClientId clientId);

  //!
  void HandleSnapshot(
    ClientId clientId,
    const protocol::RanchCommandRanchSnapshot& command);

  //!
  void HandleCmdAction(
    ClientId clientId,
    const protocol::RanchCommandRanchCmdAction& command);

  //!
  void HandleRanchStuff(
    ClientId clientId,
    const protocol::RanchCommandRanchStuff& command);

  //!
  void HandleUpdateBusyState(
    ClientId clientId,
    const protocol::RanchCommandUpdateBusyState& command);

  //!
  void HandleSearchStallion(
    ClientId clientId,
    const protocol::RanchCommandSearchStallion& command);

  //!
  void HandleEnterBreedingMarket(
    ClientId clientId,
    const protocol::RanchCommandEnterBreedingMarket& command);

  //!
  void HandleTryBreeding(
    ClientId clientId,
    const protocol::RanchCommandTryBreeding& command);

  //!
  void HandleBreedingWishlist(
    ClientId clientId,
    const protocol::RanchCommandBreedingWishlist& command);

  //!
  void HandleUpdateMountNickname(
    ClientId clientId,
    const protocol::RanchCommandUpdateMountNickname& command);

  //!
  void HandleRequestStorage(
    ClientId clientId,
    const protocol::RanchCommandRequestStorage& command);

  //!
  void HandleGetItemFromStorage(
    ClientId clientId,
    const protocol::RanchCommandGetItemFromStorage& command);

  //!
  void HandleRequestNpcDressList(
    ClientId clientId,
    const protocol::RanchCommandRequestNpcDressList& requestNpcDressList);

  void HandleChat(
    ClientId clientId,
    const protocol::RanchCommandChat& command);

  std::vector<std::string> HandleCommand(
    ClientId clientId,
    const std::string& message);

  void SendChat(
    ClientId clientId,
    const protocol::RanchCommandChatNotify& chat);

  void HandleWearEquipment(
    ClientId clientId,
    const protocol::RanchCommandWearEquipment& command);

  void HandleRemoveEquipment(
    ClientId clientId,
    const protocol::RanchCommandRemoveEquipment& command);

  void HandleCreateGuild(
    ClientId clientId,
    const protocol::RanchCommandCreateGuild& command);

  void HandleRequestGuildInfo(
    ClientId clientId,
    const protocol::RanchCommandRequestGuildInfo& command);

  void HandleUpdatePet(
    ClientId clientId,
    const protocol::RanchCommandUpdatePet& command);

  void HandleRequestPetBirth(
    ClientId clientId,
    const protocol::RanchCommandRequestPetBirth& command);

  //! Broadcasts an equipment update of the character owned by the client
  //! to the currently connected ranch.
  //! @param clientId ID of the client.
  void BroadcastEquipmentUpdate(
    ClientId clientId);

  void HandleUseItem(
    ClientId clientId,
    const protocol::RanchCommandUseItem& command);

  void HandleHousingBuild(
    ClientId clientId,
    const protocol::RanchCommandHousingBuild& command);

  void HandleOpCmd(ClientId clientId,
    const protocol::RanchCommandOpCmd& command);

  //!
  ServerInstance& _serverInstance;
  //!
  CommandServer _commandServer;
  //!
  std::unordered_map<ClientId, ClientContext> _clientContext;

  struct RanchInstance
  {
    WorldTracker _worldTracker;
    std::unordered_set<ClientId> _clients;
  };
  std::unordered_map<data::Uid, RanchInstance> _ranches;
};

} // namespace server

#endif // RANCHDIRECTOR_HPP
