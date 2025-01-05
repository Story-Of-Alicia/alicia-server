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
 */

#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <chrono>
#include <string>

class Profiler {
public:
    /**
     * @brief Constructor that starts the profiler.
     */
    Profiler();

    /**
     * @brief Starts the profiling timer.
     */
    void Start();

    /**
     * @brief Stops the profiling timer and stores the result.
     */
    void Stop();

    /**
     * @brief Retrieves the measured duration.
     * @return std::chrono::microseconds The duration measured between Start and Stop.
     */
    std::chrono::microseconds Result() const;

private:
    std::chrono::high_resolution_clock::time_point _startTime;
    std::chrono::high_resolution_clock::time_point _endTime;
    bool _isRunning;
};

#endif // PROFILER_HPP