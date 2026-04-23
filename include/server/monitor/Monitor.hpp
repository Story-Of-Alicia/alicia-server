#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <libserver/network/http/WebSocket.hpp>

namespace server
{

class ServerInstance;

class MonitorDirector
{
public:
  explicit MonitorDirector(ServerInstance& serverInstance);

  void Initialize();
  void Run();
  void Terminate();

private:
  ServerInstance& _serverInstance;
  websocket::Server _server;
};

} // namespace server

#endif // MONITOR_HPP
