//
// Created by rgnter on 14/06/2025.
//

#include "server/ServerInstance.hpp"

namespace server
{

ServerInstance::ServerInstance()
  : _lobbyDirector(*this)
  , _ranchDirector(*this)
  , _raceDirector(*this)
{
}

ServerInstance::~ServerInstance()
{
  const auto waitForThread = [](const std::string& threadName, std::thread& thread)
  {
    if (thread.joinable())
    {
      spdlog::debug("Waiting for the '{}' thread to finish...", threadName);
      thread.join();
      spdlog::debug("Thread for '{}' finished", threadName);
    }
  };

  waitForThread("race director", _raceDirectorThread);
  waitForThread("ranch director", _ranchDirectorThread);
  waitForThread("lobby director", _lobbyDirectorThread);
  waitForThread("data director", _dataDirectorThread);
}

void ServerInstance::Initialize()
{
  _shouldRun.store(true, std::memory_order::release);

  _settings.LoadFromFile("config/config.json5");
  _settings.LoadFromEnvironment();

  // Initialize the directors and tick them on their own threads.
  // Directors will terminate their tick loop once `_shouldRun` flag is set to false.

  // Data director
  _dataDirectorThread = std::thread([this]()
  {
    _dataDirector.Initialize();
    RunDirectorTaskLoop(_dataDirector);
    _dataDirector.Terminate();
  });

  // Lobby director
  _lobbyDirectorThread = std::thread([this]()
  {
    _lobbyDirector.Initialize();
    RunDirectorTaskLoop(_lobbyDirector);
    _lobbyDirector.Terminate();
  });

  // Ranch director
  _ranchDirectorThread = std::thread([this]()
  {
    _ranchDirector.Initialize();
    RunDirectorTaskLoop(_ranchDirector);
    _ranchDirector.Terminate();
  });

  // Race director
  _raceDirectorThread = std::thread([this]()
  {
    _raceDirector.Initialize();
    RunDirectorTaskLoop(_raceDirector);
    _raceDirector.Terminate();
  });

  // todo: Messenger
}

void ServerInstance::Terminate()
{
  _shouldRun.store(false, std::memory_order::relaxed);
}

DataDirector& ServerInstance::GetDataDirector()
{
  return _dataDirector;
}

LobbyDirector& ServerInstance::GetLobbyDirector()
{
  return _lobbyDirector;
}

RanchDirector& ServerInstance::GetRanchDirector()
{
  return _ranchDirector;
}

RaceDirector& ServerInstance::GetRaceDirector()
{
  return _raceDirector;
}

Settings& ServerInstance::GetSettings()
{
  return _settings;
}

} // namespace server