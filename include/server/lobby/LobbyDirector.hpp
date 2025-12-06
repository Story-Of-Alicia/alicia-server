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

#include "server/Config.hpp"

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/NetworkDefinitions.hpp>
#include <libserver/util/Scheduler.hpp>

#include <unordered_map>
#include <list>

namespace server
{

class ServerInstance;
class Config;
class LobbyNetworkHandler;

class LobbyDirector final
{
public:
  struct UserInstance
  {
    uint32_t ranchOtpCode{};
    uint32_t raceOtpCode{};

    std::string userName{};
    data::Uid characterUid{data::InvalidUid};
    data::Uid roomUid{data::InvalidUid};

    //! Only until the messenger is not implemented.
    [[deprecated]] data::Uid visitPreference{data::InvalidUid};
  };

  struct GuildInstance
  {
    std::vector<data::Uid> invites;
  };

  //! Constructor
  //! @param serverInstance Instance of the server.
  explicit LobbyDirector(ServerInstance& serverInstance);
  ~LobbyDirector();

  //! Deleted copy constructor.
  LobbyDirector(const LobbyDirector&) = delete;
  //! Deleted copy assignment.
  LobbyDirector& operator=(const LobbyDirector&) = delete;

  //! Deleted move constructor.
  LobbyDirector(LobbyDirector&&) = delete;
  //! Deleted move assignment.
  LobbyDirector& operator=(LobbyDirector&&) = delete;

  //! Initialize the director.
  void Initialize();
  //! Terminate the director.
  void Terminate();
  //! Tick the director.
  void Tick();

  bool QueueClientConnect(
    network::ClientId clientId);
  size_t QueueClientLogin(
    network::ClientId clientId,
    const std::string& userName,
    const std::string& userToken);
  size_t GetClientQueuePosition(
      network::ClientId clientId);

  void QueueClientDisconnect(
    network::ClientId clientId);
  void QueueClientLogout(
    network::ClientId clientId,
    const std::string& userName);

  bool IsUserOnline(const std::string& userName);
  UserInstance& GetUser(const std::string& userName);
  const UserInstance& GetUserByCharacterUid(data::Uid characterUid);

  void SetUserRoom(const std::string& userName, data::Uid roomUid);

  void SetCharacterForcedIntoCreator(
    data::Uid characterUid,
    bool forced);
  [[nodiscard]] bool IsCharacterForcedIntoCreator(
    data::Uid characterUid) const;

  void InviteCharacterToGuild(
    data::Uid inviteeCharacterUid,
    data::Uid guildUid,
    data::Uid inviterCharacterUid);

  // prototype function
  [[deprecated]] void SetCharacterVisitPreference(
    data::Uid characterUid,
    data::Uid rancherUid);

  void DisconnectCharacter(
    data::Uid characterUid);
  void MuteCharacter(
    data::Uid characterUid,
    data::Clock::time_point expiration);
  void NotifyCharacter(
    data::Uid characterUid,
    const std::string& message);

  //! Get users
  //! @return Get users.
  [[nodiscard]] std::unordered_map<std::string, UserInstance>& GetUsers();
  //! Get guilds
  //! @return Get guilds.
  [[nodiscard]] std::unordered_map<data::Uid, GuildInstance>& GetGuilds();

  //! Get lobby config.
  //! @return Lobby config.
  [[nodiscard]] Config::Lobby& GetConfig();
  //! Get lobby scheduler.
  //! @return Lobby scheduler.
  [[nodiscard]] Scheduler& GetScheduler();
  //! Get lobby network handler.
  //! @return Lobby network handler.
  [[nodiscard]] LobbyNetworkHandler& GetNetworkHandler();

private:
  struct QueuedLogin
  {
    //! A user name.
    std::string userName;
    //! A user token.
    std::string userToken;
    //! A flag indicating whether the load of the user was requested.
    bool userLoadRequested{false};
    //! A flag indicating whether the load of the user's character was requested.
    bool userCharacterLoadRequested{false};
  };

  std::unordered_map<network::ClientId, QueuedLogin> _clientLogins;

  std::unordered_map<std::string, UserInstance> _userInstances;
  std::unordered_map<data::Uid, GuildInstance> _guildInstances;
  std::unordered_set<data::Uid> _charactersForcedIntoCreator;

  std::list<network::ClientId> _loginRequestQueue;
  std::list<network::ClientId> _loginResponseQueue;

  //! A server instance.
  ServerInstance& _serverInstance;
  //! A scheduler.
  Scheduler _scheduler;

  //! A network handler.
  LobbyNetworkHandler* _networkHandler;
};

} // namespace server

#endif // LOBBYDIRECTOR_HPP
