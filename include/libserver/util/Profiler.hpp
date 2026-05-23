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
#include <vector>
#include <numeric>
#include <optional>

namespace server
{

/**
 * A lightweight micro-profiler for measuring operation durations.
 *
 * Collects timing samples in a fixed-size ring buffer and provides
 * basic statistics (average, min, max) over the recorded samples.
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
 *   auto avg = profiler.Average();
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

  /**
   * Constructs the profiler with a ring buffer of the given capacity.
   * @param capacity Maximum number of samples to retain. Defaults to 1000.
   */
  explicit Profiler(std::size_t capacity = 1000);
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

  /**
   * @return Average duration across all recorded samples.
   */
  [[nodiscard]] std::optional<Microseconds> Average() const;

  /**
   * @return Maximum duration across all recorded samples.
   */
  [[nodiscard]] std::optional<Microseconds> Max() const;

  /**
   * @return Minimum duration across all recorded samples.
   */
  [[nodiscard]] std::optional<Microseconds> Min() const;

  /**
   * @return Number of samples recorded, up to capacity.
   */
  [[nodiscard]] std::size_t Count() const;

private:

  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;

  mutable std::mutex _mutex;
  TimePoint _start {};
  std::size_t _count {};
  std::size_t _capacity {};
  std::size_t _index {};
  std::vector<Microseconds> _samples {};
};

} // namespace server

#endif // ALICIA_SERVER_PROFILER_HPP
