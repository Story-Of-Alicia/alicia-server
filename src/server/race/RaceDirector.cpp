//
// Created by alborrajo on 30/12/2024.
//

#include "server/race/RaceDirector.hpp"
#include "spdlog/spdlog.h"

namespace alicia
{

RaceDirector::RaceDirector(
    DataDirector& dataDirector,
    Settings::RaceSettings settings)
    : _settings(std::move(settings))
    , _dataDirector(dataDirector)
    , _server("Race")
{
    // Host the server
    _server.Host(_settings.address, _settings.port);
}


} // namespace alicia