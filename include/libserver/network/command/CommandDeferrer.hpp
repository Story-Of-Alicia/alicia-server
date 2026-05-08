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

namespace server
{

//! Class providing command deferring capability.
template <typename C>
class CommandDeferrer
{
public:
  using Handler = std::function<void(network::ClientId, const C&)>;

  //! Default constructor.
  explicit CommandDeferrer(Handler handler) noexcept
    : _handler(handler)
  {
    _command = _commands.cend();
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
    if (_command == _commands.cend())
      _command = _commands.cbegin();

    try
    {
      // Call the handler.
      _handler(_command->clientId, _command->command);
      _command = _commands.erase(_command);
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
  };

  Handler _handler;
  std::list<CommandContext> _commands;
  decltype(_commands)::const_iterator _command;
};

} // namespace server

#endif // ALICIA_SERVER_COMMANDDEFERRER_HPP
