/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#ifndef ALICIA_SERVER_COMMANDDEFERRER_HPP
#define ALICIA_SERVER_COMMANDDEFERRER_HPP

#include "libserver/network/NetworkDefinitions.hpp"

#include <functional>
#include <list>

namespace server
{

//! Class providing command deferring capability.
template <typename C>
class CommandDeferrer
{
public:
  //! Command handler taking the client ID and the command.
  //! Returns `true` if the handler completed, `false` if the handler did not complete
  //! and the execution should be deferred.
  using Handler = std::function<bool(network::ClientId, const C&)>;
  using Callback = std::function<void(network::ClientId)>;

  //! Default constructor.
  explicit CommandDeferrer(
    Handler handler,
    Callback callback,
    const size_t maxTryCount = 1) noexcept
    : _handler(handler)
    , _callback(callback)
    , _maxTryCount(maxTryCount)
  {
    _command = _commands.end();
  }

  //! Default destructor
  ~CommandDeferrer() = default;

  CommandDeferrer(const CommandDeferrer&) = delete;
  CommandDeferrer& operator=(const CommandDeferrer&) = delete;

  CommandDeferrer(CommandDeferrer&&) = delete;
  CommandDeferrer& operator=(CommandDeferrer&&) = delete;

  void Tick()
  {
    // If there's no commands skip.
    if (_commands.empty())
      return;

    // If at the end of the commands list
    // loop back to the beginning.
    if (_command == _commands.end())
    {
      _command = _commands.begin();
    }

    try
    {
      // Call the handler.
      const auto completed = _handler(_command->clientId, _command->command);
      ++_command->tryCount;

      if (completed || _command->tryCount > _maxTryCount)
      {
        _command = _commands.erase(_command);
        _callback(_command->clientId);
      }
      else
      {
        _command = _command++;
      }
    }
    catch (const std::exception& x)
    {
      _command = _commands.erase(_command);
      throw x;
    }
  }

  void Defer(const network::ClientId clientId, C command) noexcept
  {
    _commands.emplace_back(CommandContext{
      .clientId = clientId,
      .command = std::move(command)});
  }

private:
  struct CommandContext
  {
    network::ClientId clientId;
    C command;
    size_t tryCount = 0;
  };

  Handler _handler;
  Callback _callback;
  std::list<CommandContext> _commands;
  decltype(_commands)::iterator _command;
  size_t _maxTryCount = 1;
};

} // namespace server

#endif // ALICIA_SERVER_COMMANDDEFERRER_HPP
