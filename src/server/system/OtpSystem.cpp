//
// Created by rgnter on 17/07/2025.
//

#include "server/system/OtpSystem.hpp"

#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

namespace server
{

OtpSystem::OtpSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _rng(std::random_device{}())
  , _lastPurge(std::chrono::steady_clock::now())
{
}

uint32_t OtpSystem::GrantCode(const size_t key)
{
  std::scoped_lock lock(_codesMutex);

  PurgeExpired();

  const auto& settings = _serverInstance.GetSettings().otp;
  _codes.insert_or_assign(
    key,
    Code{
      .expiry = std::chrono::steady_clock::now() + settings.codeTtl,
      .authorizationAttempts = 0,
      .code = static_cast<uint32_t>(_rng())});

  return _codes.at(key).code;
}

bool OtpSystem::AuthorizeCode(const size_t key, const uint32_t code, bool consume)
{
  std::scoped_lock lock(_codesMutex);

  const auto& settings = _serverInstance.GetSettings().otp;
  const auto now = std::chrono::steady_clock::now();

  const auto codeIter = _codes.find(key);
  if (codeIter == _codes.cend())
    return false;

  Code& ctx = codeIter->second;
  const bool expired = now > ctx.expiry;
  const bool authorized = not expired && ctx.code == code;

  if (authorized)
  {
    if (consume)
      _codes.erase(codeIter);
    return true;
  }

  ctx.authorizationAttempts++;
  if (ctx.authorizationAttempts >= settings.maxFailedAttempts)
  {
    spdlog::warn("OTP key {} invalidated after {} failed attempts", key, ctx.authorizationAttempts);
    _codes.erase(codeIter);
  }
  else if (expired)
    _codes.erase(codeIter);

  return false;
}

void OtpSystem::PurgeExpired()
{
  const auto now = std::chrono::steady_clock::now();
  const auto& settings = _serverInstance.GetSettings().otp;

  if (now - _lastPurge < settings.purgeInterval)
    return;

  _lastPurge = now;

  for (auto it = _codes.begin(); it != _codes.end();)
  {
    if (now > it->second.expiry)
      it = _codes.erase(it);
    else
      ++it;
  }
}

} // namespace server
