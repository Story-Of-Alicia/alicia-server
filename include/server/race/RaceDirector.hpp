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

#include "libserver/data/DataDirector.hpp"
#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"

namespace alicia
{

class RaceDirector
{
public:
  //!
  RaceDirector(
    soa::DataDirector& dataDirector,
    Settings::RaceSettings settings = {});

  void Initialize();
  void Terminate();
  void Tick();

private:
  //!
  void HandleEnterRoom(
    ClientId clientId,
    const RaceCommandEnterRoom& enterRoom);

  //!
  void HandleChangeRoomOptions(
    ClientId clientId,
    const RaceCommandChangeRoomOptions& changeRoomOptions);

  //!
  void HandleStartRace(
    ClientId clientId,
    const RaceCommandStartRace& startRace);

  //!
  void HandleRaceTimer(
    ClientId clientId,
    const UserRaceTimer& raceTimer);

  //!
  Settings::RaceSettings _settings;
  //!
  soa::DataDirector& _dataDirector;
  //!
  CommandServer _server;

  //!
  std::unordered_map<ClientId, soa::data::Uid> _clientCharacters;

  struct RoomInstance
  {
    // Add race-specific data here
  };
  std::unordered_map<soa::data::Uid, RoomInstance> _rooms;
};

} // namespace alicia

#endif // RACEDIRECTOR_HPP
