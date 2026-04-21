//
// Created by rgnter on 30/09/2025.
//

#ifndef WEBSOCKETSERVER_HPP
#define WEBSOCKETSERVER_HPP

#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace server::websocket
{

namespace asio = boost::asio;
namespace beast = boost::beast;

class Server
{
  class Session : public std::enable_shared_from_this<Session>
  {
  public:
    using Id = size_t;

    Session(Id id, asio::ip::tcp::socket socket, Server& server);

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    void Start(std::string initialStatus);
    void Send(std::string message);

  private:
    void DoRead();
    void DoWrite();

    Id _id;
    beast::websocket::stream<beast::tcp_stream> _ws;
    beast::flat_buffer _buffer;
    Server& _server;
    std::vector<std::string> _writeQueue;
    bool _writing{false};
  };

public:
  Server() = default;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  void Listen(asio::ip::address_v4 address, uint16_t port);
  void Run();
  void Stop();

  void UpdateAndBroadcast(nlohmann::json data);
  void UpdateSection(std::string key, nlohmann::json data);

private:
  void DoAccept();
  void UnregisterSession(size_t id);

  asio::io_context _ioc;
  asio::ip::tcp::acceptor _acceptor{_ioc};
  std::unordered_map<size_t, std::shared_ptr<Session>> _sessions;
  size_t _nextSessionId{0};
  nlohmann::json _statusJson;
  std::string _lastStatus;
};

} // namespace server::websocket

#endif //WEBSOCKETSERVER_HPP
