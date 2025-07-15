//
// Created by rgnter on 14/06/2025.
//

#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "server/lobby/LobbyDirector.hpp"
#include "server/race/RaceDirector.hpp"
#include "server/ranch/RanchDirector.hpp"
#include "server/Settings.hpp"

#include "libserver/data/DataDirector.hpp"

#include <spdlog/spdlog.h>

namespace server
{

class ServerInstance final
{
public:
  ServerInstance();
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

  //! Returns reference to the settings.
  //! @returns Reference to the settings.
  Settings& GetSettings();

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

  //! A thread of the data director.
  std::thread _dataDirectorThread;
  //! A data director.
  DataDirector _dataDirector;

  //! A thread of the lobby director.
  std::thread _lobbyDirectorThread;
  //! A lobby director.
  LobbyDirector _lobbyDirector;

  //! A thread of the ranch director.
  std::thread _ranchDirectorThread;
  //! A ranch director.
  RanchDirector _ranchDirector;

  //! A thread of the race director.
  std::thread _raceDirectorThread;
  //! A race director.
  RaceDirector _raceDirector;

  //! Settings.
  Settings _settings;
};

} // namespace server

#endif //INSTANCE_HPP
