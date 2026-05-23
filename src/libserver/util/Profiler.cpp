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

#include "libserver/util/Profiler.hpp"

namespace server
{
Profiler::Profiler(std::size_t capacity)
  : _capacity(capacity)
{
  _samples.resize(capacity);
}

void Profiler::Start()
{
  _start = Clock::now();
}

void Profiler::Stop()
{
  const auto duration = std::chrono::duration_cast<Microseconds>(Clock::now() - _start);

  std::scoped_lock lock(_mutex);

  _samples.at(_index) = duration;
  _index = (_index + 1) % _capacity;
  _count = std::min(_count + 1, _capacity);
}

Profiler::ScopeGuard Profiler::Scope()
{
  return ScopeGuard(*this);
}

std::optional<Profiler::Microseconds> Profiler::Average() const
{
  std::scoped_lock lock(_mutex);

  if (_count == 0)
    return {};

  return std::reduce(_samples.begin(),
           _samples.begin() +
             static_cast<std::ptrdiff_t>(std::min(_count, _capacity)),
           Microseconds{}) /
         _count;
}

std::optional<Profiler::Microseconds> Profiler::Max() const
{
  std::scoped_lock lock(_mutex);

  if (_count == 0)
    return {};
  return *std::max_element(_samples.begin(),
    _samples.begin() +
      static_cast<std::ptrdiff_t>(std::min(_count, _capacity)));
}

std::optional<Profiler::Microseconds> Profiler::Min() const
{
  std::scoped_lock lock(_mutex);

  if (_count == 0)
    return {};
  return *std::min_element(_samples.begin(),
    _samples.begin() +
      static_cast<std::ptrdiff_t>(std::min(_count, _capacity)));
}

std::size_t Profiler::Count() const
{
  std::scoped_lock lock(_mutex);
  return _count;
}
} // namespace server
