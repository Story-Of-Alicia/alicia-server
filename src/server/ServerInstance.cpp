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

#include "server/ServerInstance.hpp"

#include <stacktrace>

namespace server
{

namespace
{
void DumpStackTrace()
{
  for (const auto& entry : std::stacktrace::current())
  {
    spdlog::error("[Stack] {}({}): {}", entry.source_file(), entry.source_line(), entry.description());
  }
}

} // anon namespace

ServerInstance::ServerInstance(
  const std::filesystem::path& resourceDirectory)
  : _resourceDirectory(resourceDirectory)
  , _authenticationService(*this)
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
  waitForThread("authentication", _authenticationThread);
}

void ServerInstance::Initialize()
{
  _shouldRun.store(true, std::memory_order::release);

  _config.LoadFromFile(_resourceDirectory / "config/server/config.yaml");
  _config.LoadFromEnvironment();

  // Read configurations

  _courseRegistry.ReadConfig(_resourceDirectory / "config/game/courses.yaml");
  _itemRegistry.ReadConfig(_resourceDirectory / "config/game/items.yaml");
  _petRegistry.ReadConfig(_resourceDirectory / "config/game/pets.yaml");

  _moderationSystem.ReadConfig(_resourceDirectory / "config/server/automod.yaml");

  // Initialize the directors and tick them on their own threads.
  // Directors will terminate their tick loop once `_shouldRun` flag is set to false.

  // Authentication service
  _authenticationThread = std::thread([this]()
  {
    try
    {
      _authenticationService.Initialize();
      RunDirectorTaskLoop(_authenticationService);
      _authenticationService.Terminate();
    }
    catch (const std::exception& x)
    {
      spdlog::error("Unhandled exception in the authentication: {}", x.what());
      DumpStackTrace();

      _shouldRun = false;
    }
  });

  // Data director
  _dataDirectorThread = std::thread([this]()
  {
    try
    {
      _dataDirector.Initialize();
      RunDirectorTaskLoop(_dataDirector);
      _dataDirector.Terminate();
    }
    catch (const std::exception& x)
    {
      spdlog::error("Unhandled exception in the data director: {}", x.what());
      DumpStackTrace();

      _shouldRun = false;
    }
  });

  // Lobby director
  _lobbyDirectorThread = std::thread([this]()
  {
    try
    {
      _lobbyDirector.Initialize();
      RunDirectorTaskLoop(_lobbyDirector);
      _lobbyDirector.Terminate();
    }
    catch (const std::exception& x)
    {
      spdlog::error("Unhandled exception in the lobby director: {}", x.what());
      DumpStackTrace();

      _shouldRun = false;
    }
  });

  // Messenger director
  if (_config.messenger.enabled)
  {
    _messengerThread = std::thread([this]()
    {
      try
      {
        _messengerDirector.Initialize();
        RunDirectorTaskLoop(_messengerDirector);
        _messengerDirector.Terminate();
      }
      catch (const std::exception& x)
      {
        spdlog::error("Unhandled exception in the messenger director: {}", x.what());
        DumpStackTrace();

        _shouldRun = false;
      }
    });

    // All chat director
    if (_config.allChat.enabled) // All chat depends on messenger
    {
      _allChatDirectorThread = std::thread([this]()
      {
        try
        {
          _allChatDirector.Initialize();
          RunDirectorTaskLoop(_allChatDirector);
          _allChatDirector.Terminate();
        }
        catch (const std::exception& x)
        {
          spdlog::error("Unhandled exception in the messenger (all chat) director: {}", x.what());
          DumpStackTrace();

          _shouldRun = false;
        }
      });
    }

    // Private chat director
    if (_config.privateChat.enabled) // Private chat depends on messenger
    {
      _privateChatDirectorThread = std::thread([this]()
      {
        try
        {
          _privateChatDirector.Initialize();
          RunDirectorTaskLoop(_privateChatDirector);
          _privateChatDirector.Terminate();
        }
        catch (const std::exception& x)
        {
          spdlog::error("Unhandled exception in the messenger (private chat) director: {}", x.what());
          DumpStackTrace();

          _shouldRun = false;
        }
      });
    }
  }

  // Ranch director
  _ranchDirectorThread = std::thread([this]()
  {
    try
    {
      _ranchDirector.Initialize();
      RunDirectorTaskLoop(_ranchDirector);
      _ranchDirector.Terminate();
    }
    catch (const std::exception& x)
    {
      spdlog::error("Unhandled exception in the ranch director: {}", x.what());
      DumpStackTrace();

      _shouldRun = false;
    }
  });

  // Race director
  _raceDirectorThread = std::thread([this]()
  {
    try
    {
      _raceDirector.Initialize();
      RunDirectorTaskLoop(_raceDirector);
      _raceDirector.Terminate();
    }
    catch (const std::exception& x)
    {
      spdlog::error("Unhandled exception in the race director: {}", x.what());
      DumpStackTrace();

      _shouldRun = false;
    }
  });
}

void ServerInstance::Terminate()
{
  _shouldRun.store(false, std::memory_order::relaxed);
}

AuthenticationService& ServerInstance::GetAuthenticationService()
{
  return _authenticationService;
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