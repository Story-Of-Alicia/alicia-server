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

#include <chrono>
#include <functional>
#include <list>
#include <mutex>

namespace server
{

//!
class Scheduler final
{
public:
  //! A task to perform.
  using Task = std::function<void()>;
  //! An alias for the standard steady-clock.
  using Clock = std::chrono::steady_clock;

  Scheduler();

  //! Tick the scheduler.
  void Tick();

  //! Queue a task to be executed in the next tick.
  //! @param task Task to queue execution of.
  //! @param when A time point of when to execute the task. Defaults to immediate execution.
  void Queue(
    const Task& task,
    Clock::time_point when = Clock::now());

protected:
  //! A job.
  struct Job
  {
    //! A time point of when the job should execute.
    Clock::time_point when{};
    //! A task the job has to execute.
    Task task{};
  };

  //! A mutex to the job list.
  std::mutex _jobsMutex;
  //! A job list.
  std::list<Job> _jobs;
  //! An iterator to the job to execute in the next tick cycle.
  decltype(_jobs)::const_iterator _jobIterator;
};

} // namespace server

#endif // SERVER_SCHEDULER_HPP
