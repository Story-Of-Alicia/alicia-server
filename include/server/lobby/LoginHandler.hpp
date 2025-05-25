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

#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP

#include "libserver/data/DataDirector.hpp"
#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/LobbyMessageDefinitions.hpp"

namespace alicia
{
class LobbyDirector;

//! Login handler.
class LoginHandler
{
public:
  LoginHandler(
    LobbyDirector& lobbyDirector,
    CommandServer& server);

  void Tick();

  //!
  void HandleUserLogin(
    ClientId clientId,
    const LobbyCommandLogin& login);

private:
  struct LoginContext
  {
    ClientId clientId;
    std::string userName;
    std::string userToken;
  };

  void QueueUserLoginAccepted(ClientId clientId, const std::string& userName);
  void QueueUserCreateNickname(ClientId clientId, const std::string& userName);
  void QueueUserLoginRejected(ClientId clientId);

  std::unordered_map<ClientId, LoginContext> _clientLogins;
  std::queue<ClientId> _clientLoginRequestQueue;
  std::queue<ClientId> _clientLoginResponseQueue;

  //!
  CommandServer& _server;
  //!
  LobbyDirector& _lobbyDirector;
  soa::DataDirector& _dataDirector;
};

} // namespace alicia

#endif // LOGINHANDLER_HPP
