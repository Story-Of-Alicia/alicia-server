//
// Created by maros.prejsa on 08/05/2026.
//

#ifndef ALICIA_SERVER_COMMANDDEFERRER_HPP
#define ALICIA_SERVER_COMMANDDEFERRER_HPP

#include "libserver/network/NetworkDefinitions.hpp"

#include <functional>

namespace server
{

constexpr size_t DeferredCommandMaxTries = 6;

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
