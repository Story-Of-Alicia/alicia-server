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

#ifndef LOBBYDIRECTOR_HPP
#define LOBBYDIRECTOR_HPP

#include "LoginHandler.hpp"

#include "server/Settings.hpp"

#include "libserver/data/DataDefinitions.hpp"
#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/LobbyMessageDefinitions.hpp"

#include <unordered_map>

namespace server
{

class ServerInstance;
class Settings;

class LobbyDirector final
  : public CommandServer::EventHandlerInterface
{
  friend LoginHandler;

public:
  //!
  explicit LobbyDirector(ServerInstance& serverInstance);

  LobbyDirector(const LobbyDirector&) = delete;
  LobbyDirector& operator=(const LobbyDirector&) = delete;

  LobbyDirector(LobbyDirector&&) = delete;
  LobbyDirector& operator=(LobbyDirector&&) = delete;

  void Initialize();
  void Terminate();
  void Tick();

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId clientId) override;

  ServerInstance& GetServerInstance();
  Settings::LobbySettings& GetSettings();

  void UpdateVisitPreference(data::Uid characterUid, data::Uid visitingCharacterUid);

private:
  void HandleEnterChannel(
    ClientId clientId,
    const protocol::LobbyCommandEnterChannel& command);

  void HandleRoomList(
    ClientId,
    const protocol::LobbyCommandRoomList& command);

  void HandleMakeRoom(
    ClientId clientId,
    const protocol::LobbyCommandMakeRoom& command);

  void HandleEnterRoom(
    ClientId clientId,
    const protocol::LobbyCommandEnterRoom& command);

  //!
  void HandleHeartbeat(
    ClientId clientId,
    const protocol::LobbyCommandHeartbeat& command);

  //!
  void HandleShowInventory(
    ClientId clientId,
    const protocol::LobbyCommandShowInventory& command);

  void QueueShowInventory(
    ClientId clientId);

  //!
  void HandleAchievementCompleteList(
    ClientId clientId,
    const protocol::LobbyCommandAchievementCompleteList& command);

  //!
  void HandleRequestLeagueInfo(
    ClientId clientId,
    const protocol::LobbyCommandRequestLeagueInfo& command);

  //!
  void HandleRequestQuestList(
    ClientId clientId,
    const protocol::LobbyCommandRequestQuestList& command);

  //!
  void HandleRequestDailyQuestList(
    ClientId clientId,
    const protocol::LobbyCommandRequestDailyQuestList& command);

  //!
  void HandleRequestSpecialEventList(
    ClientId clientId,
    const protocol::LobbyCommandRequestSpecialEventList& command);

  //!
  void HandleRequestPersonalInfo(
    ClientId clientId,
    const protocol::LobbyCommandRequestPersonalInfo& command);

  void HandleSetIntroduction(
    ClientId clientId,
    const protocol::LobbyCommandSetIntroduction& command);

  //!
  void HandleEnterRanch(
    ClientId clientId,
    const protocol::LobbyCommandEnterRanch& command);

  void QueueEnterRanchOK(
    ClientId clientId,
    data::Uid rancherUid);

  //!
  void HandleGetMessengerInfo(
    ClientId clientId,
    const protocol::LobbyCommandGetMessengerInfo& command);

  //!
  void HandleGoodsShopList(
    ClientId clientId,
    const protocol::LobbyCommandGoodsShopList& command);

  //!
  void HandleInquiryTreecash(
    ClientId clientId,
    const protocol::LobbyCommandInquiryTreecash& command);

  //!
  void HandleGuildPartyList(
    ClientId clientId,
    const protocol::LobbyCommandGuildPartyList& command);

  void HandleUpdateSystemContent(
    ClientId clientId,
    const protocol::LobbyCommandUpdateSystemContent& command);

  //!
  ServerInstance& _serverInstance;
  //!
  CommandServer _commandServer;
  //!
  LoginHandler _loginHandler;

protected:
  struct ClientContext
  {
    //! Whether the client is authorized.
    bool isAuthorized{false};
    data::Uid characterUid = data::InvalidUid;
    data::Uid rancherVisitPreference = data::InvalidUid;
  };

  protocol::LobbyCommandLoginOK::SystemContent _systemContent{
    .values = {{4, 0}, {16, 1}}};
  
  //!
  std::unordered_map<ClientId, ClientContext> _clientContext;
};

} // namespace server

#endif // LOBBYDIRECTOR_HPP
