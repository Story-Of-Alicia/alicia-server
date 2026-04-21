//
// Created by rgnter on 30/09/2025.
//

#include "libserver/network/http/WebSocket.hpp"

#include <spdlog/spdlog.h>

namespace server::websocket
{

// --- Session ---

Server::Session::Session(Id id, asio::ip::tcp::socket socket, Server& server)
  : _id(id)
  , _ws(std::move(socket))
  , _server(server)
{
}

void Server::Session::Start(std::string initialStatus)
{
  _ws.set_option(
    beast::websocket::stream_base::timeout::suggested(beast::role_type::server));
  _ws.set_option(beast::websocket::stream_base::decorator(
    [](beast::websocket::response_type& res)
    {
      res.set(beast::http::field::server, "alicia-monitor");
    }));

  _ws.async_accept(
    [self = shared_from_this(), initial = std::move(initialStatus)](
      beast::error_code ec) mutable
    {
      if (ec)
      {
        spdlog::warn("Monitor WS handshake error (id={}): {}", self->_id, ec.message());
        self->_server.UnregisterSession(self->_id);
        return;
      }
      self->_ws.text(true);
      if (!initial.empty())
        self->Send(std::move(initial));
      self->DoRead();
    });
}

void Server::Session::Send(std::string message)
{
  _writeQueue.push_back(std::move(message));
  if (!_writing)
    DoWrite();
}

void Server::Session::DoRead()
{
  _ws.async_read(
    _buffer,
    [self = shared_from_this()](beast::error_code ec, size_t)
    {
      if (ec)
      {
        if (ec != beast::websocket::error::closed)
          spdlog::debug("Monitor client (id={}) disconnected: {}", self->_id, ec.message());
        self->_server.UnregisterSession(self->_id);
        return;
      }
      self->_buffer.consume(self->_buffer.size());
      self->DoRead();
    });
}

void Server::Session::DoWrite()
{
  if (_writeQueue.empty())
  {
    _writing = false;
    return;
  }
  _writing = true;
  _ws.async_write(
    asio::buffer(_writeQueue.front()),
    [self = shared_from_this()](beast::error_code ec, size_t)
    {
      if (ec)
      {
        spdlog::warn("Monitor client write error (id={}): {}", self->_id, ec.message());
        self->_server.UnregisterSession(self->_id);
        return;
      }
      self->_writeQueue.erase(self->_writeQueue.begin());
      self->DoWrite();
    });
}

// --- Server ---

void Server::Listen(asio::ip::address_v4 address, uint16_t port)
{
  const asio::ip::tcp::endpoint endpoint(address, port);
  beast::error_code ec;

  _acceptor.open(endpoint.protocol(), ec);
  if (ec)
  {
    spdlog::error("Monitor WS open error: {}", ec.message());
    return;
  }
  _acceptor.set_option(asio::socket_base::reuse_address(true), ec);
  _acceptor.bind(endpoint, ec);
  if (ec)
  {
    spdlog::error("Monitor WS bind error: {}", ec.message());
    return;
  }
  _acceptor.listen(asio::socket_base::max_listen_connections, ec);
  if (ec)
  {
    spdlog::error("Monitor WS listen error: {}", ec.message());
    return;
  }

  spdlog::info("Monitor WebSocket listening on {}:{}", address.to_string(), port);
  DoAccept();
}

void Server::Run()
{
  _ioc.run();
}

void Server::Stop()
{
  asio::post(_ioc, [this]()
  {
    beast::error_code ec;
    _acceptor.close(ec);
    _sessions.clear();
  });
  _ioc.stop();
}

void Server::UpdateAndBroadcast(nlohmann::json data)
{
  auto msg = std::make_shared<std::string>(data.dump());
  asio::post(_ioc, [this, msg]()
  {
    _lastStatus = *msg;
    for (auto& [id, session] : _sessions)
      session->Send(*msg);
  });
}

void Server::UpdateSection(std::string key, nlohmann::json data)
{
  auto k = std::make_shared<std::string>(std::move(key));
  auto d = std::make_shared<nlohmann::json>(std::move(data));
  asio::post(_ioc, [this, k, d]()
  {
    _statusJson[*k] = std::move(*d);
    _lastStatus = _statusJson.dump();
    for (auto& [id, session] : _sessions)
      session->Send(_lastStatus);
  });
}

void Server::DoAccept()
{
  _acceptor.async_accept(
    [this](beast::error_code ec, asio::ip::tcp::socket socket)
    {
      if (ec)
      {
        if (ec != asio::error::operation_aborted)
          spdlog::warn("Monitor WS accept error: {}", ec.message());
        return;
      }
      const size_t id = _nextSessionId++;
      auto session = std::make_shared<Session>(id, std::move(socket), *this);
      _sessions.emplace(id, session);
      session->Start(_lastStatus);
      DoAccept();
    });
}

void Server::UnregisterSession(size_t id)
{
  spdlog::debug("Monitor client disconnected (id={})", id);
  _sessions.erase(id);
}

} // namespace server::websocket
