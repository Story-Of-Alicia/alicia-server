//
// Created by alborrajo on 16/11/2024.
//

#ifndef RANCHDIRECTOR_HPP
#define RANCHDIRECTOR_HPP

#include "server/Settings.hpp"
#include "server/tracker/WorldTracker.hpp"

#include "libserver/data/DataDirector.hpp"
#include "libserver/command/CommandServer.hpp"

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

}

#endif //RANCHDIRECTOR_HPP
