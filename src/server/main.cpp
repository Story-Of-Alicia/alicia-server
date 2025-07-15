#include "Version.hpp"

#include "server/ServerInstance.hpp"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <memory>

#ifdef WIN32
  #include <windows.h>
#else
  #include <signal.h>
#endif

namespace
{

using Clock = std::chrono::steady_clock;

std::atomic_bool shouldProgramRun = true;

std::shared_ptr<spdlog::logger> g_logger;
Clock::time_point serverStartupTime;

#ifdef WIN32

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
  switch (fdwCtrlType)
  {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
      {
        spdlog::debug("Shutting down because of CTRL+C");
        shouldProgramRun.store(false, std::memory_order::relaxed);
        return TRUE;
      }

    default:
      return FALSE;
  }
}
#else

void handler(int sig, siginfo_t* info, void* ucontext)
{
  if (sig == SIGTERM)
  {
    spdlog::debug("Shutting down because of SIGTERM");
    shouldProgramRun.store(false, std::memory_order::relaxed);
  }
}

#endif

} // anon namespace

int main()
{
#ifdef WIN32
  // Register the control handler.
  if (SetConsoleCtrlHandler(CtrlHandler, TRUE) == FALSE)
  {
    spdlog::error(
      "Failed to set the console control handler. Windows error: 0x{:x}",
      GetLastError());
    return 1;
  };
#else
  // Register the signal action handler.
  struct sigaction act{};

  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = &handler;
  if (sigaction(SIGTERM, &act, nullptr) == -1)
  {
    spdlog::error("Failed to change the signal action handler for SIGTERM");
    return 1;
  }
#endif

  serverStartupTime = std::chrono::steady_clock::now();

  // Daily file sink.
  const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
    "logs/log.log", 0, 0);

  // Console sink.
  const auto consoleSink = std::make_shared<
    spdlog::sinks::stdout_color_sink_mt>();

  // Initialize the application logger with file sink and console sink.
  g_logger = std::make_shared<spdlog::logger>(
    "server",
    spdlog::sinks_init_list{fileSink, consoleSink});

  g_logger->set_level(spdlog::level::debug);
  g_logger->set_pattern("%H:%M:%S:%e [%^%l%$] [Thread %t] %v");

  // Set is as the default logger for the application.
  spdlog::set_default_logger(g_logger);

  spdlog::info("Running dedicated Alicia server v{}.", server::BuildVersion);

  server::ServerInstance serverInstance;
  serverInstance.Initialize();

  spdlog::info(
    "Server started up in {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(
      Clock::now() - serverStartupTime)
      .count());

  while (shouldProgramRun)
  {
    std::string commandLine;
    std::getline(std::cin, commandLine);

    const auto command = server::util::TokenizeString(
      commandLine, ' ');

    if (command[0] == "exit")
    {
      shouldProgramRun.exchange(false, std::memory_order::relaxed);
    }
    else if (command[0] == "register")
    {
      if (command.size() < 3)
      {
        printf("Please specify name and token\n");
        continue;
      }
    }
  }

  serverInstance.Terminate();

  return 0;
}
