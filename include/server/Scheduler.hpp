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

#ifndef SERVER_SCHEDULER_HPP
#define SERVER_SCHEDULER_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

namespace server
{

using Task = std::function<void()>;

//!
class TaskLoop final
{
public:
  //! Begin the task loop.
  void Begin();
  //! End the task loop.
  void End();
  //! Queue a task.
  void Queue(const Task& task);

protected:
  //! Task queue mutex.
  std::mutex _queueMutex;
  //! Task queue notification.
  std::condition_variable _queueNotification;
  //! Task queue.
  std::queue<Task> _queue;
  //! A flag indicating whether the task loop should run.
  std::atomic<bool> _shouldRun;
};

} // namespace server

#endif // SERVER_SCHEDULER_HPP
