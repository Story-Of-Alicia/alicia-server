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
  , _allChatDirector(*this)
  , _privateChatDirector(*this)
  , _ranchDirector(*this)
  , _raceDirector(*this)
  , _chatSystem(*this)
  , _infractionSystem(*this)
  , _itemSystem(*this)
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
  waitForThread("private chat director", _privateChatDirectorThread);
  waitForThread("all chat director", _allChatDirectorThread);
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
  _magicRegistry.ReadConfig(_resourceDirectory / "config/game/magic.yaml");
  _petRegistry.ReadConfig(_resourceDirectory / "config/game/pets.yaml");

  _moderationSystem.ReadConfig(_resourceDirectory / "config/server/automod.yaml");

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
  if (_config.messenger.enabled)
  {
    _messengerThread = std::thread([this]()
    {
      _messengerDirector.Initialize();
      RunDirectorTaskLoop(_messengerDirector);
      _messengerDirector.Terminate();
    });

    // All chat director
    if (_config.allChat.enabled) // All chat depends on messenger
    {
      _allChatDirectorThread = std::thread([this]()
      {
        _allChatDirector.Initialize();
        RunDirectorTaskLoop(_allChatDirector);
        _allChatDirector.Terminate();
      });
    }

    // Private chat director
    if (_config.privateChat.enabled) // Private chat depends on messenger
    {
      _privateChatDirectorThread = std::thread([this]()
      {
        _privateChatDirector.Initialize();
        RunDirectorTaskLoop(_privateChatDirector);
        _privateChatDirector.Terminate();
      });
    }
  }

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

MessengerDirector& ServerInstance::GetMessengerDirector()
{
  return _messengerDirector;
}

AllChatDirector& ServerInstance::GetAllChatDirector()
{
  return _allChatDirector;
}

PrivateChatDirector& ServerInstance::GetPrivateChatDirector()
{
  return _privateChatDirector;
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

registry::MagicRegistry& ServerInstance::GetMagicRegistry()
{
  return _magicRegistry;
}

ChatSystem& ServerInstance::GetChatSystem()
{
  return _chatSystem;
}

InfractionSystem& ServerInstance::GetInfractionSystem()
{
  return _infractionSystem;
}

ItemSystem& ServerInstance::GetItemSystem()
{
  return _itemSystem;
}

ModerationSystem& ServerInstance::GetModerationSystem()
{
  return _moderationSystem;
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