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

#include "libserver/util/Scheduler.hpp"

namespace server
{

Scheduler::Scheduler()
{
  _jobIterator = _jobs.cend();
}

void Scheduler::Tick()
{
  // Make sure there is jobs to execute.
  if (_jobs.empty())
    return;

  // If the job iterator is at the end cycle it back to the front,
  // since the job list is not empty.
  if (_jobIterator == _jobs.cend())
  {
    _jobIterator = _jobs.begin();
  }

  // Iterate over the jobs until you find one that can be executed,
  // execute it and remove it from the job list.
  while (true)
  {
    const auto& job = *_jobIterator;
    if (Clock::now() >= job.when)
    {
      job.task();
      _jobIterator = _jobs.erase(_jobIterator);
      break;
    }

    if (++_jobIterator == _jobs.cend())
      break;
  }
}

void Scheduler::Queue(
  const Task& task,
  const Clock::time_point when)
{
  std::scoped_lock lock(_jobsMutex);
  _jobs.emplace_back(Job{
    .when = when,
    .task = task});
}


} // namespace server
