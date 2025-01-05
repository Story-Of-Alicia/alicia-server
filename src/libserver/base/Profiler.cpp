#include "Profiler.hpp"

Profiler::Profiler() : _isRunning(false) {}

void Profiler::Start() {
    _isRunning = true;
    _startTime = std::chrono::high_resolution_clock::now();
}

void Profiler::Stop() {
    if (_isRunning) {
        _endTime = std::chrono::high_resolution_clock::now();
        _isRunning = false;
    }
}

std::chrono::microseconds Profiler::Result() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(_endTime - _startTime);
}