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

#include "server/Scheduler.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>

#include <spdlog/spdlog.h>

namespace server
{

void TaskLoop::Begin()
{
  _shouldRun = true;

  while (true)
  {
    std::unique_lock queueLock(_queueMutex);

    while (!_queue.empty())
    {
      // Poll the task from the queue.
      std::function<void()> task;
      task = _queue.front();
      _queue.pop();

      // Process the task.
      task();
    }

    if (_queue.empty())
    {
      if (!_shouldRun)
      {
        // Break the task loop if the task queue is empty
        // and the task loop should not run.
        break;
      }
    }

    // Wait for queue notification.
    _queueNotification.wait(queueLock);
  }
}

void TaskLoop::End()
{
  _shouldRun = false;
  _queueNotification.notify_all();
}

void TaskLoop::Queue(const Task& task)
{
  {
    std::scoped_lock queueLock(_queueMutex);
    _queue.emplace(task);
  }

  _queueNotification.notify_all();
}

} // namespace server
