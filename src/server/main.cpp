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

#include "Version.hpp"
#include "server/ServerInstance.hpp"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <memory>

#ifdef WIN32
  #include <windows.h>
#else
  #include <unistd.h>
  #include <signal.h>
#endif

namespace
{

using Clock = std::chrono::steady_clock;

std::atomic_bool shouldProgramRun = true;
std::condition_variable shouldProgramRunCv;

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
        shouldProgramRunCv.notify_all();
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
    shouldProgramRunCv.notify_all();
  }
}

#endif

void InteractiveLoop()
{
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
  }
}

} // anon namespace

int main(int argc, char** argv)
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

  std::filesystem::path resourceDirectory;

  // todo: parse arguments
  std::vector<std::string> arguments;
  for (int32_t idx = 1; idx < argc; ++idx)
  {
    arguments.emplace_back(argv[idx]);
  }

  for (const auto& argument : arguments | std::views::join_with(' '))
  {
    resourceDirectory += argument;
  }

  spdlog::info("Running dedicated Alicia server v{}.", server::BuildVersion);
  if (not resourceDirectory.empty())
    spdlog::info("Resources directory: {}", resourceDirectory.string());
  else
    spdlog::info("Resources directory is the working directory");

  server::ServerInstance serverInstance(resourceDirectory);
  serverInstance.Initialize();

  spdlog::info(
    "Server started up in {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(
      Clock::now() - serverStartupTime)
      .count());

  bool interactiveMode = true;

  #ifndef WIN32
  if (not isatty(STDIN_FILENO))
  {
    spdlog::info("TTY not available");
    interactiveMode = false;
  }
  #endif

  if (not interactiveMode)
  {
    spdlog::info("Not running in an interactive mode");

    std::mutex threadMtx;
    std::unique_lock threadLock(threadMtx);

    while (shouldProgramRun)
    {
      shouldProgramRunCv.wait(threadLock);
    }
  }
  else
  {
    InteractiveLoop();
  }

  serverInstance.Terminate();

  return 0;
}
