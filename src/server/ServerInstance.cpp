//
// Created by rgnter on 14/06/2025.
//

#include "server/ServerInstance.hpp"

namespace server
{

ServerInstance::ServerInstance(
  const std::filesystem::path& resourceDirectory)
  : _resourceDirectory(resourceDirectory)
  , _dataDirector(resourceDirectory / "data")
  , _lobbyDirector(*this)
  , _messengerDirector(*this)
  , _ranchDirector(*this)
  , _raceDirector(*this)
  , _chatSystem(*this)
  , _infractionSystem(*this)
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
  waitForThread("messenger director", _messengerThread);
  waitForThread("lobby director", _lobbyDirectorThread);
  waitForThread("data director", _dataDirectorThread);
}

void ServerInstance::Initialize()
{
  _shouldRun.store(true, std::memory_order::release);

  _config.LoadFromFile(_resourceDirectory / "config/server/config.yaml");
  _config.LoadFromEnvironment();

  // Read configurations

  _courseRegistry.ReadConfig(_resourceDirectory / "config/game/courses.yaml");
  _itemRegistry.ReadConfig(_resourceDirectory / "config/game/items.yaml");

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

  // Messenger director
  _messengerThread = std::thread([this]()
  {
    _messengerDirector.Initialize();
    RunDirectorTaskLoop(_messengerDirector);
    _messengerDirector.Terminate();
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

registry::CourseRegistry& ServerInstance::GetCourseRegistry()
{
  return _courseRegistry;
}

registry::HorseRegistry& ServerInstance::GetHorseRegistry()
{
  return _horseRegistry;
}

registry::ItemRegistry& ServerInstance::GetItemRegistry()
{
  return _itemRegistry;
}

registry::PetRegistry& ServerInstance::GetPetRegistry()
{
  return _petRegistry;
}

ChatSystem& ServerInstance::GetChatSystem()
{
  return _chatSystem;
}

InfractionSystem& ServerInstance::GetInfractionSystem()
{
  return _infractionSystem;
}

RoomSystem& ServerInstance::GetRoomSystem()
{
  return _roomSystem;
}

OtpSystem& ServerInstance::GetOtpSystem()
{
  return _otpSystem;
}

Config& ServerInstance::GetSettings()
{
  return _config;
}

} // namespace server