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

#ifndef RACEDIRECTOR_HPP
#define RACEDIRECTOR_HPP

#include "server/Settings.hpp"

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"
#include "server/tracker/WorldTracker.hpp"

#include <unordered_map>
#include <vector>

namespace server
{
class ServerInstance;
} // namespace server

namespace server
{

class RaceDirector final
  : public CommandServer::EventHandlerInterface
{
public:
  //!
  explicit RaceDirector(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId clientId) override;

  ServerInstance& GetServerInstance();
  Settings::RaceSettings& GetSettings();

private:
  //!
  void HandleEnterRoom(
    ClientId clientId,
    const protocol::RaceCommandEnterRoom& command);

  //!
  void HandleChangeRoomOptions(
    ClientId clientId,
    const protocol::RaceCommandChangeRoomOptions& command);

  //!
  void HandleStartRace(
    ClientId clientId,
    const protocol::RaceCommandStartRace& command);

  //!
  void HandleRaceTimer(
    ClientId clientId,
    const protocol::RaceCommandUserRaceTimer& command);

  void HandleReadyRace(
    ClientId clientId,
    const protocol::RaceCommandReadyRace& command);

  //!
  ServerInstance& _serverInstance;
  //!
  CommandServer _commandServer;

  struct ClientContext
  {
    data::Uid characterUid{data::InvalidUid};
    data::Uid roomUid{data::InvalidUid};
    bool ready = false;
  };
  std::unordered_map<ClientId, ClientContext> _clientContexts;

  struct Room
  {
    std::vector<ClientId> clients;
    WorldTracker worldTracker;
  };
  std::unordered_map<uint32_t, Room> _roomInstances;
};

} // namespace server

#endif // RACEDIRECTOR_HPP
