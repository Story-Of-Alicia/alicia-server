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
  _networkHandler->Initialize();
}

void LobbyDirector::Terminate()
{
  _networkHandler->Terminate();
}

void LobbyDirector::Tick()
{
  while (not _loginResponseQueue.empty())
  {
    const network::ClientId clientId = _loginResponseQueue.front();
    auto& loginContext = _clientLogins[clientId];

    // If the user character load was already requested wait for the load to complete.
    if (loginContext.userCharacterLoadRequested)
    {
      if (_serverInstance.GetDataDirector().AreDataBeingLoaded(loginContext.userName))
      {
        continue;
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
        continue;
      }
    }

    _loginResponseQueue.pop_front();

    const bool forcedCharacterCreator = _charactersForcedIntoCreator.erase(
      characterUid) > 0;

    // If the user does not have a character or the character creator was enforced
    // send them to the character creator.
    if (not hasCharacter || forcedCharacterCreator)
    {
      spdlog::debug(
        "User '{}' (client {}) succeeded in authentication and was sent to the character creator",
        loginContext.userName,
        clientId);
      _networkHandler->AcceptLogin(clientId, true);
      return;
    }

    // If the character was not loaded reject the login.
    if (not _serverInstance.GetDataDirector().AreCharacterDataLoaded(
      loginContext.userName))
    {
      spdlog::error("User character data for '{}' not available", clientId);
      _networkHandler->RejectLogin(
        clientId,
        protocol::AcCmdCLLoginCancel::Reason::Generic);
      break;
    }

    const auto& [iter, inserted] = _userInstances.try_emplace(
      loginContext.userName);
    if (not inserted)
    {
      _networkHandler->RejectLogin(
        clientId,
        protocol::AcCmdCLLoginCancel::Reason::Duplicated);
      return;
    }

    spdlog::debug("User '{}' succeeded in authentication", loginContext.userName);
    _networkHandler->AcceptLogin(clientId);

    auto& userInstance = iter->second;
    userInstance.userName = loginContext.userName;
    userInstance.characterUid = characterUid;
    spdlog::info("User '{}' (client {}) logged in", loginContext.userName, clientId);

    _clientLogins.erase(clientId);

    // Only one response per tick.
    break;
  }

  // Process the client login request queue.
  while (not _loginRequestQueue.empty())
  {
    const network::ClientId clientId = _loginRequestQueue.front();
    auto& loginContext = _clientLogins[clientId];

    // Request the load of the user data if not requested yet.
    if (not loginContext.userLoadRequested)
    {
      _serverInstance.GetDataDirector().RequestLoadUserData(loginContext.userName);

      loginContext.userLoadRequested = true;
      continue;
    }

    // If the data are still being loaded do not proceed with login.
    if (_serverInstance.GetDataDirector().AreDataBeingLoaded(loginContext.userName))
    {
      continue;
    }

    _loginRequestQueue.pop_front();

    if (not _serverInstance.GetDataDirector().AreUserDataLoaded(loginContext.userName))
    {
      spdlog::error("User data for '{}' not available", loginContext.userName);
      _networkHandler->RejectLogin(
        clientId,
        protocol::AcCmdCLLoginCancel::Reason::Generic);
      break;
    }

    const auto userRecord = _serverInstance.GetDataDirector().GetUser(loginContext.userName);
    assert(userRecord.IsAvailable());

    bool isAuthenticated = false;
    userRecord.Immutable(
      [&isAuthenticated, &loginContext](const data::User& user)
      {
        isAuthenticated = user.token() == loginContext.userToken;
      });

    // If the user is not authenticated reject the login.
    if (not isAuthenticated)
    {
      spdlog::debug("User '{}' failed in authentication", loginContext.userName);
      _networkHandler->RejectLogin(
        clientId,
        protocol::AcCmdCLLoginCancel::Reason::InvalidUser);
    }
    else
    {
      // Check for any infractions preventing the user from joining.
      const auto infractionVerdict = _serverInstance.GetInfractionSystem().CheckOutstandingPunishments(
        loginContext.userName);

      if (infractionVerdict.preventServerJoining)
      {
        _networkHandler->RejectLogin(
          clientId,
          protocol::AcCmdCLLoginCancel::Reason::DisconnectYourself);
      }
      else
      {
        // Queue the user response.
        _loginResponseQueue.emplace_back(clientId);
      }
    }

    // Only one request per tick.
    break;
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

LobbyNetworkHandler& LobbyDirector::GetNetworkHandler()
{
  return *_networkHandler;
}

} // namespace server