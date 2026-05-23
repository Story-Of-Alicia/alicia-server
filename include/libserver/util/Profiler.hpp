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

#ifndef ALICIA_SERVER_PROFILER_HPP
#define ALICIA_SERVER_PROFILER_HPP

#include <chrono>
#include <mutex>
#include <optional>

namespace server
{

/**
 * A lightweight micro-profiler for measuring operation durations.
 *
 * Records timing samples using std::chrono::steady_clock.
 * Supports both manual Start()/Stop() calls and RAII-based scoped measurement.
 *
 * @example
 * @code
 *   Profiler profiler;
 *
 *   // Manual usage:
 *   profiler.Start();
 *   doWork();
 *   profiler.Stop();
 *
 *   // Scoped usage:
 *   { auto guard = profiler.Scope(); doWork(); }
 *
 *   auto result = profiler.Result();
 * @endcode
 */
class Profiler
{
public:
  using Microseconds = std::chrono::microseconds;

  /**
   * RAII wrapper that calls Start() on construction and Stop() on destruction.
   * Non-copyable to prevent double-recording.
   */
  struct ScopeGuard
  {
    explicit ScopeGuard(Profiler & profile) : profile(profile)
    {
      profile.Start();
    }

    ~ScopeGuard()
    {
      profile.Stop();
    }

    ScopeGuard(const ScopeGuard &) = delete;
    ScopeGuard & operator=(const ScopeGuard &) = delete;

    Profiler & profile;
  };

  Profiler() = default;
  ~Profiler() = default;

  /** Begins a timing measurement. */
  void Start();

  /** Ends the current timing measurement and records the elapsed duration. */
  void Stop();

  /**
   * Returns a ScopeGuard that times the enclosing scope automatically.
   * @return A ScopeGuard bound to this profiler.
   */
  ScopeGuard Scope();

  /** Returns the most recently recorded sample duration, or empty if no samples have been recorded.
   * @return The most recent sample
   */
  std::optional<Microseconds> Result() const;

private:

  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;

  mutable std::mutex _mutex;
  TimePoint _start {};
  std::optional<Microseconds> _lastSample;
};

} // namespace server

#endif // ALICIA_SERVER_PROFILER_HPP
