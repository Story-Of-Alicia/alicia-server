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

#include "server/lobby/LobbyDirector.hpp"

#include "server/lobby/LobbyNetworkHandler.hpp"
#include "server/ServerInstance.hpp"

namespace server
{

LobbyDirector::LobbyDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _networkHandler(new LobbyNetworkHandler(_serverInstance))
{
}

LobbyDirector::~LobbyDirector()
{
  delete _networkHandler;
}

void LobbyDirector::Initialize()
{
  _shopManager.GenerateShopList(_serverInstance.GetItemRegistry());
  _networkHandler->Initialize();
}

void LobbyDirector::Terminate()
{
  _networkHandler->Terminate();
}

void LobbyDirector::Tick()
{
  // Process the client login response queue.
  if (not _loginResponseQueue.empty())
  {
    ProcesLoginResponse();
  }

  // Process the client login request queue.
  if (not _loginRequestQueue.empty())
  {
    ProcessLoginRequest();
  }

  if (_serverInstance.GetAuthenticationService().HasAuthenticationVerdicts())
  {
    const auto authentications = _serverInstance.GetAuthenticationService().PollAuthenticationVerdicts();

    for (auto& loginContext : _clientLogins |  std::views::values)
    {
      for (const auto& authenticationVerdict : authentications)
      {
        if (authenticationVerdict.userName != loginContext.userName)
          continue;

        loginContext.isAuthenticated = authenticationVerdict.isAuthenticated;
      }
    }
  }

  _scheduler.Tick();
}

bool LobbyDirector::QueueClientConnect(network::ClientId clientId)
{
  const auto [iter, inserted] = _clientLogins.try_emplace(clientId);
  if (not inserted)
    return false;
  return true;
}

size_t LobbyDirector::QueueClientLogin(
  network::ClientId clientId,
  const std::string& userName,
  const std::string& userToken)
{
  const auto clientLoginIter = _clientLogins.find(clientId);
  if (clientLoginIter == _clientLogins.cend())
    return 99;

  clientLoginIter->second.userName = userName;
  clientLoginIter->second.userToken = userToken;

  _loginRequestQueue.emplace_back(clientId);

  return _loginRequestQueue.size() + _loginResponseQueue.size();
}

size_t LobbyDirector::GetClientQueuePosition(
  network::ClientId clientId)
{
  size_t position{0};

  // Distance in the login response queue
  const auto responseIter = std::ranges::find(_loginResponseQueue, clientId);
  if (responseIter != _loginResponseQueue.cend())
    position += std::ranges::distance(_loginResponseQueue.begin(), responseIter);

  // Distance in the login request queue
  const auto requestIter = std::ranges::find(_loginRequestQueue, clientId);
  if (requestIter != _loginRequestQueue.cend())
    position += std::ranges::distance(_loginRequestQueue.begin(), requestIter);

  return position;
}

void LobbyDirector::QueueClientDisconnect(
  network::ClientId clientId)
{
  _loginRequestQueue.remove(clientId);
  _loginResponseQueue.remove(clientId);
  _clientLogins.erase(clientId);
}

void LobbyDirector::QueueClientLogout(
  [[maybe_unused]] network::ClientId clientId,
  const std::string& userName)
{
  spdlog::info("User '{}' (client {}) logged out", userName, clientId);
  _userInstances.erase(userName);
}

bool LobbyDirector::IsUserOnline(const std::string& userName)
{
  return _userInstances.contains(userName);
}

LobbyDirector::UserInstance& LobbyDirector::GetUser(
  const std::string& userName)
{
  const auto iter = _userInstances.find(userName);
  if (iter == _userInstances.cend())
  {
    throw std::runtime_error(
      std::format(
        "User instance '{}' not available",
        userName));
  }

  return iter->second;
}

const LobbyDirector::UserInstance& LobbyDirector::GetUserByCharacterUid(
  data::Uid characterUid)
{
  for (const auto& userInstance : _userInstances | std::views::values)
  {
    if (userInstance.characterUid == characterUid)
      return userInstance;
  }

  throw std::runtime_error(
    std::format(
      "User instance for character {} not available",
      characterUid));
}

void LobbyDirector::SetUserRoom(const std::string& userName, data::Uid roomUid)
{
  const auto userIter = _userInstances.find(userName);
  if (userIter == _userInstances.cend())
    return;

  userIter->second.roomUid = roomUid;
}

void LobbyDirector::SetCharacterForcedIntoCreator(
  const data::Uid characterUid,
  const bool forced)
{
  if (forced)
    _charactersForcedIntoCreator.emplace(characterUid);
  else
    _charactersForcedIntoCreator.erase(characterUid);
}

bool LobbyDirector::IsCharacterForcedIntoCreator(
  const data::Uid characterUid) const
{
  return _charactersForcedIntoCreator.contains(characterUid);
}

void LobbyDirector::InviteCharacterToGuild(
  const data::Uid inviteeCharacterUid,
  const data::Uid guildUid,
  const data::Uid inviterCharacterUid)
{
  _guildInstances[guildUid].invites.emplace_back(inviteeCharacterUid);

  _networkHandler->SendCharacterGuildInvitation(
    inviteeCharacterUid,
    guildUid,
    inviterCharacterUid);
}

void LobbyDirector::SetCharacterVisitPreference(
  const data::Uid characterUid,
  const data::Uid rancherUid)
{
  _networkHandler->SetCharacterVisitPreference(characterUid, rancherUid);
}

void LobbyDirector::DisconnectCharacter(
  const data::Uid characterUid)
{
  _networkHandler->DisconnectCharacter(characterUid);
}

void LobbyDirector::MuteCharacter(
  const data::Uid characterUid,
  const data::Clock::time_point expiration)
{
  _networkHandler->MuteCharacter(characterUid, expiration);
}

void LobbyDirector::NotifyCharacter(
  const data::Uid characterUid,
  const std::string& message)
{
  _networkHandler->NotifyCharacter(characterUid, message);
}

std::unordered_map<std::string, LobbyDirector::UserInstance>& LobbyDirector::GetUsers()
{
  return _userInstances;
}

std::unordered_map<data::Uid, LobbyDirector::GuildInstance>& LobbyDirector::GetGuilds()
{
  return _guildInstances;
}

Config::Lobby& LobbyDirector::GetConfig()
{
  return _serverInstance.GetSettings().lobby;
}

Scheduler& LobbyDirector::GetScheduler()
{
  return _scheduler;
}

ShopManager& LobbyDirector::GetShopManager()
{
  return _shopManager;
}

LobbyNetworkHandler& LobbyDirector::GetNetworkHandler()
{
  return *_networkHandler;
}

void LobbyDirector::ProcessLoginRequest()
{
  const network::ClientId clientId = _loginRequestQueue.front();
  auto& loginContext = _clientLogins[clientId];

  // Request authentication of the user if not yet requested.
  if (not loginContext.userAuthenticationRequested)
  {
    _serverInstance.GetAuthenticationService().QueueAuthentication(
      loginContext.userName,
      loginContext.userToken);

    loginContext.userAuthenticationRequested = true;
    return;
  }

  // If the authentication result is not available skip to the next user.
  if (not loginContext.isAuthenticated.has_value())
    return;

  if (not loginContext.isAuthenticated.value())
  {
    spdlog::info("User '{}' failed authentication", loginContext.userName);
    _networkHandler->RejectLogin(
      clientId,
      protocol::AcCmdCLLoginCancel::Reason::InvalidUser);
    _loginRequestQueue.pop_front();
    return;
  }

  // Request the load of the user data if not requested yet.
  if (not loginContext.userLoadRequested)
  {
    _serverInstance.GetDataDirector().RequestLoadUserData(loginContext.userName);

    loginContext.userLoadRequested = true;
    return;
  }

  // If the data are still being loaded do not proceed with login.
  if (_serverInstance.GetDataDirector().AreDataBeingLoaded(loginContext.userName))
  {
    return;
  }

  _loginRequestQueue.pop_front();

  if (not _serverInstance.GetDataDirector().AreUserDataLoaded(loginContext.userName))
  {
    spdlog::error("User data for '{}' are not available", loginContext.userName);
    _networkHandler->RejectLogin(
      clientId,
      protocol::AcCmdCLLoginCancel::Reason::Generic);
    spdlog::warn("Rejected login of user '{}' because of a server error", loginContext.userName);
    return;
  }

  const auto userRecord = _serverInstance.GetDataDirector().GetUser(
    loginContext.userName);
  assert(userRecord.IsAvailable());

  // Check for any infractions preventing the user from joining.
  const auto infractionVerdict = _serverInstance.GetInfractionSystem().CheckOutstandingPunishments(
    loginContext.userName);

  if (infractionVerdict.preventServerJoining)
  {
    _networkHandler->RejectLogin(
      clientId,
      protocol::AcCmdCLLoginCancel::Reason::DisconnectYourself);
    spdlog::info("Rejected login of user '{}' because of an infraction", loginContext.userName);
  }
  else
  {
    spdlog::info("Accepted login of user '{}'", loginContext.userName);
    // Queue the user response.
    _loginResponseQueue.emplace_back(clientId);
  }
}

void LobbyDirector::ProcesLoginResponse()
{
  const network::ClientId clientId = _loginResponseQueue.front();
  auto& loginContext = _clientLogins[clientId];

  // If the user character load was already requested wait for the load to complete.
  if (loginContext.userCharacterLoadRequested)
  {
    if (_serverInstance.GetDataDirector().AreDataBeingLoaded(loginContext.userName))
    {
      return;
    }
  }

  const auto userRecord = _serverInstance.GetDataDirector().GetUser(loginContext.userName);
  assert(userRecord.IsAvailable());

  auto characterUid = data::InvalidUid;
  userRecord.Immutable(
    [&characterUid](const data::User& user)
    {
      characterUid = user.characterUid();
    });

  const bool hasCharacter = characterUid != data::InvalidUid;

  // If the user has a character request the load.
  if (hasCharacter)
  {
    // If the user character is not loaded do not proceed.
    if (not loginContext.userCharacterLoadRequested)
    {
      _serverInstance.GetDataDirector().RequestLoadCharacterData(
        loginContext.userName,
        characterUid);

      loginContext.userCharacterLoadRequested = true;
      return;
    }
  }

  _loginResponseQueue.pop_front();

  // If the character was not loaded reject the login.
  if (characterUid != data::InvalidUid && not _serverInstance.GetDataDirector().AreCharacterDataLoaded(
    loginContext.userName))
  {
    spdlog::error("User character data for '{}' not available", clientId);
    _networkHandler->RejectLogin(
      clientId,
      protocol::AcCmdCLLoginCancel::Reason::Generic);
    spdlog::warn("Rejected login of user '{}' because of a server error", loginContext.userName);
    return;
  }

  const auto& [iter, inserted] = _userInstances.try_emplace(
    loginContext.userName);
  if (not inserted)
  {
    _networkHandler->RejectLogin(
      clientId,
      protocol::AcCmdCLLoginCancel::Reason::Duplicated);
    spdlog::warn(
      "Rejected login of user '{}' because the user is already logged in from different location",
      loginContext.userName);
    return;
  }

  const bool requiresCharacterCreator = _charactersForcedIntoCreator.erase(characterUid) > 0
    || characterUid == data::InvalidUid;

  _networkHandler->AcceptLogin(clientId, requiresCharacterCreator);

  auto& userInstance = iter->second;
  userInstance.userName = loginContext.userName;
  userInstance.characterUid = characterUid;
  spdlog::info("User '{}' (client {}) logged in", loginContext.userName, clientId);

  _clientLogins.erase(clientId);
}

} // namespace server