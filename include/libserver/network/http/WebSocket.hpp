//
// Created by rgnter on 30/09/2025.
//

#ifndef WEBSOCKETSERVER_HPP
#define WEBSOCKETSERVER_HPP

#include <nlohmann/json.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast.hpp>

#include <queue>

namespace server::websocket
{

namespace asio = boost::asio;
namespace beast = boost::beast;

class Client
{

};

class Server
{
  class Session
  {
  public:
    enum class Id : size_t{};

    explicit Session(asio::ip::tcp::socket socket);
    ~Session() = default;

    Session(const Session&) = delete;
    void operator=(const Session&) = delete;

    Session(Session&&) = delete;
    void operator=(Session&&) = delete;

    void QueueData(const nlohmann::json& data);

  private:
    asio::streambuf _incomingBuffer;
    asio::streambuf _outgoingBuffer;
    std::queue<nlohmann::json> _queue;
    asio::ip::tcp::socket _socket;
  };

  class EventHandler
  {
    virtual ~EventHandler() = default;

    virtual void OnSessionConnected(Session& session);
    virtual void OnSessionDisconnected(Session& session);
    virtual void OnSessionDataReceived(Session& session) = 0;
  };

public:
  void Host();
  void GetSession(Session::Id id);

private:
  asio::io_context _ioContext{};
  std::unordered_map<Session::Id, Session> _sessions;
};

} // namespace server::websocket

#endif //WEBSOCKETSERVER_HPP
