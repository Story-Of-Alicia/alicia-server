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

#ifndef ALICIA_SERVER_AUTHORIZATIONSERVICE_HPP
#define ALICIA_SERVER_AUTHORIZATIONSERVICE_HPP

#include "AuthenticationBackend.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <queue>

namespace server
{

class ServerInstance;

class AuthenticationService final
{
public:
  struct Verdict
  {
    std::string userName;
    bool isAuthenticated{false};
  };

  explicit AuthenticationService(ServerInstance& serverInstance);

  void Initialize();
  void Terminate() noexcept;
  void Tick() noexcept;

  //! Thread safe
  void QueueAuthentication(
    const std::string& userName,
    const std::string& userToken) noexcept;

  //! Thread safe
  [[nodiscard]] bool HasAuthenticationVerdicts() noexcept;

  //! Thread safe
  [[nodiscard]] std::vector<Verdict> PollAuthenticationVerdicts() noexcept;

private:
  struct Authentication
  {
    std::string userName;
    std::string userToken;
  };

  ServerInstance& _serverInstance;

  std::mutex _queueMutex;
  std::queue<Authentication> _queue{};

  std::atomic_bool _hasVerdicts{false};
  std::mutex _verdictsMutex{};
  std::vector<Verdict> _verdicts{};

  std::unique_ptr<AuthenticationBackend> _backend;
};

} // namespace server

#endif // ALICIA_SERVER_AUTHORIZATIONSERVICE_HPP
