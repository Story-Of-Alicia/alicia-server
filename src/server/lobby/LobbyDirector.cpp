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
    const std::string& userName = _loginResponseQueue.front();
    auto& loginContext = _userLogins[userName];

    // If the user character load was already requested wait for the load to complete.
    if (loginContext.userCharacterLoadRequested)
    {
      if (_serverInstance.GetDataDirector().AreDataBeingLoaded(userName))
      {
        continue;
      }
    }

    const auto userRecord = _serverInstance.GetDataDirector().GetUser(userName);
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
          userName,
          characterUid);

        loginContext.userCharacterLoadRequested = true;
        continue;
      }
    }

    _loginResponseQueue.pop();

    const bool forcedCharacterCreator = _charactersForcedIntoCreator.erase(
      characterUid) > 0;

    // If the user does not have a character or the character creator was enforced
    // send them to the character creator.
    if (not hasCharacter || forcedCharacterCreator)
    {
      loginContext.justCreatedCharacter = true;

      spdlog::debug("User '{}' sent to the character creator", userName);
      _networkHandler->SendUserToCharacterCreator(userName);
      return;
    }

    // If the character was not loaded reject the login.
    if (not _serverInstance.GetDataDirector().AreCharacterDataLoaded(
      userName))
    {
      spdlog::error("User character data for '{}' not available", userName);
      _networkHandler->SendUserLoginRejected(
        userName,
        protocol::AcCmdCLLoginCancel::Reason::Generic);
      break;
    }

    const auto& [iter, inserted] = _userInstances.try_emplace(userName);
    if (not inserted)
    {
      _networkHandler->SendUserLoginRejected(
        userName,
        protocol::AcCmdCLLoginCancel::Reason::Duplicated);
      return;
    }

    auto& userInstance = iter->second;
    userInstance.userName = userName;
    userInstance.characterUid = characterUid;

    spdlog::debug("User '{}' succeeded in authentication", userName);
    _networkHandler->SendUserLoginAccepted(userName);

    // Only one response per tick.
    break;
  }

  // Process the client login request queue.
  while (not _loginRequestQueue.empty())
  {
    const std::string& userName = _loginRequestQueue.front();
    auto& loginContext = _userLogins[userName];

    // Request the load of the user data if not requested yet.
    if (not loginContext.userLoadRequested)
    {
      _serverInstance.GetDataDirector().RequestLoadUserData(
        userName);

      loginContext.userLoadRequested = true;
      continue;
    }

    // If the data are still being loaded do not proceed with login.
    if (_serverInstance.GetDataDirector().AreDataBeingLoaded(
      userName))
    {
      continue;
    }

    _loginRequestQueue.pop();

    if (not _serverInstance.GetDataDirector().AreUserDataLoaded(
      userName))
    {
      spdlog::error("User data for '{}' not available", userName);
      _networkHandler->SendUserLoginRejected(userName, protocol::AcCmdCLLoginCancel::Reason::Generic);
      break;
    }

    const auto userRecord = _serverInstance.GetDataDirector().GetUser(
      userName);
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
      spdlog::debug("User '{}' failed in authentication", userName);
      _networkHandler->SendUserLoginRejected(userName, protocol::AcCmdCLLoginCancel::Reason::InvalidUser);
    }
    else
    {
      // Check for any infractions preventing the user from joining.
      const auto infractionVerdict = _serverInstance.GetInfractionSystem().CheckOutstandingPunishments(
        userName);

      if (infractionVerdict.preventServerJoining)
      {
        _networkHandler->SendUserLoginRejected(
          userName,
          protocol::AcCmdCLLoginCancel::Reason::DisconnectYourself);
      }
      else
      {
        // Queue the user response.
        _loginResponseQueue.emplace(userName);
      }
    }

    // Only one request per tick.
    break;
  }

  _scheduler.Tick();
}

void LobbyDirector::QueueUserLogin(
  const std::string& userName,
  const std::string& userToken)
{
  {
    std::scoped_lock lock(_userLoginsMutex);
    const auto [iter, inserted] = _userLogins.try_emplace(userName);
    if (not inserted)
      return;

    iter->second.userToken = userToken;
  }

  {
    std::scoped_lock lock(_loginRequestQueueMutex);
    _loginRequestQueue.emplace(userName);
  }
}

void LobbyDirector::QueueCharacterCreated(
  const std::string& userName)
{
  std::scoped_lock lock(_loginResponseQueueMutex);
  _loginRequestQueue.emplace(userName);
}

size_t LobbyDirector::GetUserQueuePosition(
  const std::string& userName)
{
  // todo: make the queue a list
  //       count response queue position + count request queue position
  return 0;
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
