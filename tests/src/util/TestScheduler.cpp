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

#include <libserver/util/Scheduler.hpp>

#include <array>
#include <cassert>

namespace
{

void TestSequencedTasks()
{
  constexpr uint32_t TaskCount = 12;

  server::Scheduler scheduler;

  // Sequence tasks to execute.
  std::array<bool, TaskCount> taskResults{};
  for (uint32_t taskIdx = 0; taskIdx < TaskCount; ++taskIdx)
  {
    scheduler.Queue([&taskResults, taskIdx]()
    {
      taskResults[taskIdx] = true;
    });
  }

  // Expect the task to finish in an order they were submitted in.
  for (uint32_t tickIdx = 0; tickIdx < TaskCount; ++tickIdx)
  {
    scheduler.Tick();
    assert(taskResults[tickIdx]);
  }
}

void TestScheduledTasks()
{
  constexpr auto DelayDuration = std::chrono::seconds(1);

  bool delayedTaskExecuted = false;
  bool immediateTaskExecuted = false;

  server::Scheduler scheduler;
  scheduler.Queue([&delayedTaskExecuted](){
    delayedTaskExecuted = true;
  }, server::Scheduler::Clock::now() + DelayDuration);

  scheduler.Queue([&immediateTaskExecuted](){
    immediateTaskExecuted = true;
  });

  // Perform one tick, expect the immediate job to be executed.
  scheduler.Tick();
  assert(immediateTaskExecuted);
  assert(not delayedTaskExecuted);

  // Perform ticks until the delayed job completes or until the test timeout.
  auto timeout = server::Scheduler::Clock::now() + DelayDuration * 2;
  while (true)
  {
    scheduler.Tick();
    if (delayedTaskExecuted)
      break;
    if (server::Scheduler::Clock::now() > timeout
      || delayedTaskExecuted)
    {
      break;
    }
  }

  assert(delayedTaskExecuted && "Task queued for execution with a delay not executed within a timeout");
}

} // namespace

int main()
{
  TestSequencedTasks();
  TestScheduledTasks();
}
