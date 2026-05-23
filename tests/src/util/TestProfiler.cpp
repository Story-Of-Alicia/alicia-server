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

  assert(profiler.Count() == 0 && "Count should be zero with no samples");
  assert(not profiler.Average().has_value() && "Average should be empty with no samples");
  assert(not profiler.Min().has_value() && "Min should be empty with no samples");
  assert(not profiler.Max().has_value() && "Max should be empty with no samples");
}

void TestSingleSample()
{
  server::Profiler profiler;

  profiler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  profiler.Stop();

  assert(profiler.Count() == 1 && "Count should be one after a single sample");
  assert(profiler.Average().has_value() && "Average should have a value after a sample");
  assert(profiler.Average().value().count() > 0 && "Average duration should be greater than zero");
  assert(profiler.Min() == profiler.Max() && "Min and Max should be equal with one sample");
}

void TestMultipleSamples()
{
  server::Profiler profiler;

  for (int i = 0; i < 5; ++i)
  {
    profiler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    profiler.Stop();
  }

  assert(profiler.Count() == 5 && "Count should match number of recorded samples");
  assert(profiler.Min().value() <= profiler.Average().value() && "Min should be less than or equal to Average");
  assert(profiler.Average().value() <= profiler.Max().value() && "Average should be less than or equal to Max");
}

void TestScopeGuard()
{
  server::Profiler profiler;

  {
    auto guard = profiler.Scope();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  assert(profiler.Count() == 1 && "ScopeGuard should record exactly one sample");
  assert(profiler.Average().value().count() > 0 && "ScopeGuard sample duration should be greater than zero");
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

  assert(profiler.Count() == 1 && "ScopeGuard should record a sample even when an exception is thrown");
  assert(profiler.Average().value().count() > 0 && "ScopeGuard duration should be valid after exception");
}

void TestRingBufferWraparound()
{
  constexpr std::size_t Capacity = 5;
  server::Profiler profiler(Capacity);

  // Record more samples than the capacity.
  for (int i = 0; i < 10; ++i)
  {
    profiler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    profiler.Stop();
  }

  // Count should be capped at capacity.
  assert(profiler.Count() == Capacity && "Count should not exceed ring buffer capacity");
}

void TestCustomCapacity()
{
  constexpr std::size_t Capacity = 3;
  server::Profiler profiler(Capacity);

  for (int i = 0; i < 3; ++i)
  {
    profiler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    profiler.Stop();
  }

  assert(profiler.Count() == Capacity && "Count should match custom capacity when full");
  assert(profiler.Average().has_value() && "Average should have a value when buffer is full");
  assert(profiler.Min().has_value() && "Min should have a value when buffer is full");
  assert(profiler.Max().has_value() && "Max should have a value when buffer is full");
}

void TestMinMaxOrdering()
{
  server::Profiler profiler;

  // Short sample.
  profiler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  profiler.Stop();

  // Longer sample.
  profiler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  profiler.Stop();

  assert(profiler.Count() == 2 && "Count should be two after two samples");
  assert(profiler.Min().value() < profiler.Max().value() && "Min should be less than Max with different durations");
}

} // namespace

int main()
{
  TestNoSamples();
  TestSingleSample();
  TestMultipleSamples();
  TestScopeGuard();
  TestScopeGuardOnException();
  TestRingBufferWraparound();
  TestCustomCapacity();
  TestMinMaxOrdering();
}