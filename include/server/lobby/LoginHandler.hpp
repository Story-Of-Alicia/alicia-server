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

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/LobbyMessageDefinitions.hpp"

#include <chrono>
#include <unordered_map>

namespace server
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
    const protocol::LobbyCommandLogin& login);

  //!
  void HandleUserCreateCharacter(
    ClientId clientId,
    const protocol::LobbyCommandCreateNickname& command);

  //!
  void HandleUserDisconnect(ClientId clientId);

  void QueueUserLoginAccepted(ClientId clientId, const std::string& userName);
  void QueueUserCreateNickname(ClientId clientId, const std::string& userName);
  void QueueUserLoginRejected(ClientId clientId, bool invalidUser = false);

private:
  using Clock = std::chrono::steady_clock;

  struct LoginContext
  {
    ClientId clientId;
    std::string userName;
    std::string userToken;

    //! Whether the load of the user was requested.
    bool userLoadRequested{false};
    //! Whether the load of the user's character was requested.
    bool userCharacterLoadRequested{false};
  };

  std::unordered_map<ClientId, LoginContext> _clientLogins;

  std::mutex _clientLoginRequestQueueMutex;
  std::queue<ClientId> _clientLoginRequestQueue;
  std::mutex _clientLoginResponseQueueMutex;
  std::queue<ClientId> _clientLoginResponseQueue;

  //!
  CommandServer& _server;
  //!
  LobbyDirector& _lobbyDirector;
};

} // namespace server

#endif // LOGINHANDLER_HPP
