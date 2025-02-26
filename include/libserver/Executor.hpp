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

#include <functional>
#include <cstdint>
#include <thread>
#include <atomic>
#include <queue>

namespace server
{

class Executor
{
public:
  /**
   * Starts the execution loop.
   */
  Executor(uint64_t const timeout);

  /**
   * Stops the execution loop, blocks until all tasks are finished.
   */
  ~Executor();

  /**
   * Executes the provided function in the Executor's
   * execution queue.
   */
  void execute(std::function<void()> const & function);

private:
  std::atomic<bool> _shouldRun = true;
  std::atomic<bool> _notification = false;

  std::thread _executor;

  std::mutex  _queueMutex;
  std::queue<std::function<void()>> _queue;
};
} // namespace server

#endif // SERVER_SCHEDULER_HPP
