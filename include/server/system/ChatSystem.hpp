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

#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <functional>
#include <string>
#include <span>
#include <unordered_map>

namespace server
{

class ServerInstance;

class CommandManager
{
public:
  //! Generic command handler callback.
  //! @param arguments Command arguments
  //! @param characterUid UID of the character that invoked the command.
  //! @returns Response
  using Handler = std::function<std::vector<std::string>(
    const std::span<const std::string>& arguments,
    data::Uid characterUid)>;

  //! Registers a command handler for a literal.
  void RegisterCommand(const std::string& literal, Handler handler) noexcept;

  [[nodiscard]] std::vector<std::string> HandleCommand(
    const std::string& literal,
    data::Uid characterUid, const
    std::span<const std::string>& arguments) noexcept;

private:
  std::unordered_map<std::string, Handler> _commands;
};

class ChatSystem
{
public:
  explicit ChatSystem(ServerInstance& dataDirector);
  ~ChatSystem();

  struct CommandVerdict
  {
    std::vector<std::string> result;
  };

  struct ChatVerdict
  {
    std::string message;
    std::optional<CommandVerdict> commandVerdict;
  };

  //! Handles a chat message sent by the client.
  //! @param characterUid UID of the character.
  //! @param message Message that was sent.
  [[nodiscard]] ChatVerdict ProcessChatMessage(
    data::Uid characterUid,
    const std::string& message) noexcept;

  [[nodiscard]] CommandVerdict ProcessCommandMessage(
    data::Uid characterUid,
    const std::string& message);

private:
  void RegisterUserCommands();
  void RegisterAdminCommands();

  void Broadcast(std::string message);

  //! A server instance.
  ServerInstance& _serverInstance;
  //! A command manager.
  CommandManager _commandManager;
};

} // namespace server

#endif //COMMANDHANDLER_HPP
