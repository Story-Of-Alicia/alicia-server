//
// Created by rgnter on 26/11/2024.
//

#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP

#include "libserver/data/DataDirector.hpp"
#include "libserver/command/CommandServer.hpp"

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

#endif //LOGINHANDLER_HPP
