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

#ifndef RANCHDIRECTOR_HPP
#define RANCHDIRECTOR_HPP

#include "server/Settings.hpp"
#include "server/tracker/WorldTracker.hpp"

#include "libserver/data/DataDirector.hpp"
#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"

namespace alicia
{

class RanchDirector
{
public:
  //!
  explicit RanchDirector(
    soa::DataDirector& dataDirector,
    Settings::RanchSettings settings = {});

  void Initialize();
  void Terminate();
  void Tick();

private:
  //!
  void HandleEnterRanch(
    ClientId clientId,
    const RanchCommandEnterRanch& enterRanch);

  //!
  void HandleSnapshot(
    ClientId clientId,
    const RanchCommandRanchSnapshot& snapshot);

  //!
  void HandleCmdAction(
    ClientId clientId,
    const RanchCommandRanchCmdAction& action);

  //!
  void HandleRanchStuff(
    ClientId clientId,
    const RanchCommandRanchStuff& command);

  //!
  void HandleUpdateBusyState(
    ClientId clientId,
    const RanchCommandUpdateBusyState& command);

  //!
  void HandleSearchStallion(
    ClientId clientId,
    const RanchCommandSearchStallion& command);

  //!
  void HandleEnterBreedingMarket(
    ClientId clientId,
    const RanchCommandEnterBreedingMarket& command);

  //!
  void HandleTryBreeding(
    ClientId clientId,
    const RanchCommandTryBreeding& command);

  //!
  void HandleBreedingWishlist(
    ClientId clientId,
    const RanchCommandBreedingWishlist& command);

  //!
  void HandleUpdateMountNickname(
    ClientId clientId,
    const RanchCommandUpdateMountNickname& command);

  //!
  void HandleRequestStorage(
    ClientId clientId,
    const RanchCommandRequestStorage& command);

  //!
  void HandleRequestNpcDressList(
    ClientId clientId,
    const RanchCommandRequestNpcDressList& requestNpcDressList);

  //!
  Settings::RanchSettings _settings;
  //!
  soa::DataDirector& _dataDirector;
  //!
  CommandServer _server;

  //!
  std::unordered_map<ClientId, std::string> _clientUsers;

  struct RanchInstance
  {
    WorldTracker _worldTracker;
  };
  std::unordered_map<soa::data::Uid, RanchInstance> _ranches;
};

} // namespace alicia

#endif // RANCHDIRECTOR_HPP
