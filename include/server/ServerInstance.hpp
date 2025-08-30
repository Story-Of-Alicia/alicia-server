//
// Created by rgnter on 14/06/2025.
//

#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "server/Config.hpp"
#include "server/lobby/LobbyDirector.hpp"
#include "server/messenger/MessengerDirector.hpp"
#include "server/race/RaceDirector.hpp"
#include "server/ranch/RanchDirector.hpp"
#include "server/system/ChatSystem.hpp"

#include <libserver/data/DataDirector.hpp>
#include <libserver/registry/HorseRegistry.hpp>
#include <libserver/registry/OtpRegistry.hpp>

#include <spdlog/spdlog.h>

namespace server
{

class ServerInstance final
{
public:
  //! Constructor.
  //! @param resourceDirectory Directory for server resources.
  explicit ServerInstance(const std::filesystem::path& resourceDirectory);
  ~ServerInstance();

  //! Initializes the server instance.
  void Initialize();
  //! Terminates the server instance.
  void Terminate();

  //! Returns reference to the data director.
  //! @returns Reference to the data director.
  DataDirector& GetDataDirector();

  //! Returns reference to the lobby director.
  //! @returns Reference to the lobby director.
  LobbyDirector& GetLobbyDirector();

  //! Returns reference to the ranch director.
  //! @returns Reference to the ranch director.
  RanchDirector& GetRanchDirector();

  //! Returns reference to the race director.
  //! @returns Reference to the race director.
  RaceDirector& GetRaceDirector();

  //! Returns reference to the chat system.
  //! @returns Reference to the chat system.
  ChatSystem& GetChatSystem();

  //! Returns reference to the settings.
  //! @returns Reference to the settings.
  Config& GetSettings();

  //! Returns reference to the OTP registry.
  //! @returns Reference to the OTP registry.
  OtpRegistry& GetOtpRegistry();

private:

  template<typename T>
  void RunDirectorTaskLoop(T& director)
  {
    using Clock = std::chrono::steady_clock;

    constexpr float TicksPerSecond = 50;
    constexpr uint64_t millisPerTick = 1000ull / TicksPerSecond;

    Clock::time_point lastTick;
    while (_shouldRun.load(std::memory_order::relaxed))
    {
      const auto timeNow = Clock::now();
      // Time delta between ticks [ms].
      const auto tickDelta = std::chrono::duration_cast<
        std::chrono::milliseconds>(timeNow - lastTick);

      if (tickDelta < std::chrono::milliseconds(millisPerTick))
      {
        const auto sleepMs = millisPerTick - tickDelta.count();
        std::this_thread::sleep_for(
          std::chrono::milliseconds(sleepMs));
        continue;
      }

      lastTick = timeNow;

      try
      {
        director.Tick();
      }
      catch (const std::exception& x)
      {
        spdlog::error("Exception in tick loop: {}", x.what());
        break;
      }
    }
  }

  //! Atomic flag indicating whether the server should run.
  std::atomic_bool _shouldRun{false};

  //! A path to the resource directory.
  std::filesystem::path _resourceDirectory;
  //! A config.
  Config _config;

  //! A thread of the data director.
  std::thread _dataDirectorThread;
  //! A data director.
  DataDirector _dataDirector;

  //! A thread of the lobby director.
  std::thread _lobbyDirectorThread;
  //! A lobby director.
  LobbyDirector _lobbyDirector;

  //! A thread for the messenger director.
  std::thread _messengerThread;
  //! A messenger director.
  MessengerDirector _messengerDirector;

  //! A thread of the ranch director.
  std::thread _ranchDirectorThread;
  //! A ranch director.
  RanchDirector _ranchDirector;

  //! A thread of the race director.
  std::thread _raceDirectorThread;
  //! A race director.
  RaceDirector _raceDirector;


  ChatSystem _chatSystem;

  //! Registry of OTP codes.
  OtpRegistry _otpRegistry;
};

} // namespace server

#endif //INSTANCE_HPP
