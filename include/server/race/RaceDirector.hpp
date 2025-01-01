//
// Created by alborrajo on 30/12/2024.
//

#ifndef RACEDIRECTOR_HPP
#define RACEDIRECTOR_HPP

#include "server/DataDirector.hpp"
#include "server/Settings.hpp"

#include "libserver/command/CommandServer.hpp"

namespace alicia
{

class RaceDirector
{
public:
  //!
  RaceDirector(
      DataDirector& dataDirector,
      Settings::RaceSettings settings = {});

private:

  //!
  Settings::RaceSettings _settings;
  //!
  DataDirector& _dataDirector;
  //!
  CommandServer _server;

  //!
  std::unordered_map<ClientId, DatumUid> _clientCharacters;

  struct RoomInstance
  {
    // Add race-specific data here
  };
  std::unordered_map<DatumUid, RoomInstance> _rooms;
};

}

#endif //RACEDIRECTOR_HPP