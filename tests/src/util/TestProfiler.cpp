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

#include <libserver/util/Profiler.hpp>

#include <cassert>
#include <thread>

namespace
{

void TestNoSamples()
{
  server::Profiler profiler;
  assert(not profiler.Result().has_value() && "Result should be empty with no samples");
}

void TestStartStop()
{
  server::Profiler profiler;
  profiler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  profiler.Stop();
  assert(profiler.Result().has_value() && "Result should have a value after Stop");
  assert(profiler.Result().value().count() > 0 && "Duration should be greater than zero");
}

void TestScopeGuard()
{
  server::Profiler profiler;

  {
    auto guard = profiler.Scope();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  assert(profiler.Result().value().count() > 0 && "ScopeGuard sample duration should be greater than zero");
}

void TestScopeGuardOnException()
{
  server::Profiler profiler;

  try
  {
    auto guard = profiler.Scope();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    throw std::runtime_error("test");
  }
  catch (...)
  {
  }

  assert(profiler.Result().value().count() > 0 && "ScopeGuard duration should be valid after exception");
}


} // namespace

int main()
{
  TestNoSamples();
  TestStartStop();
  TestScopeGuard();
  TestScopeGuardOnException();
}