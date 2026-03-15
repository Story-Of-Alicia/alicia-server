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
      .code = static_cast<uint32_t>(_rng())});

  // Reset failed attempts when a new code is granted for this key,
  // since the legitimate server is issuing a fresh code.
  _attempts.erase(key);

  return _codes.at(key).code;
}

bool OtpSystem::AuthorizeCode(const size_t key, const uint32_t code, bool consume)
{
  std::scoped_lock lock(_codesMutex);

  const auto& settings = _serverInstance.GetSettings().otp;
  const auto now = std::chrono::steady_clock::now();

  // Check brute-force lockout.
  if (const auto attemptIter = _attempts.find(key); attemptIter != _attempts.end())
  {
    const auto& record = attemptIter->second;
    if (record.failedAttempts >= settings.maxFailedAttempts && now < record.lockoutExpiry)
    {
      spdlog::warn("OTP authorization for key {} denied: locked out after {} failed attempts",
        key,
        record.failedAttempts);
      return false;
    }
  }

  const auto codeIter = _codes.find(key);
  if (codeIter == _codes.cend())
  {
    auto& record = _attempts[key];
    record.failedAttempts++;
    if (record.failedAttempts >= settings.maxFailedAttempts)
      record.lockoutExpiry = now + settings.lockoutDuration;
    return false;
  }

  const Code& ctx = codeIter->second;

  const bool expired = now > ctx.expiry;
  const bool authorized = not expired && ctx.code == code;

  if (authorized)
  {
    if (consume)
      _codes.erase(codeIter);

    _attempts.erase(key);
  }
  else
  {
    auto& record = _attempts[key];
    record.failedAttempts++;

    if (record.failedAttempts >= settings.maxFailedAttempts)
    {
      record.lockoutExpiry = now + settings.lockoutDuration;
      spdlog::warn("OTP key {} locked out for {}s after {} failed attempts",
        key,
        settings.lockoutDuration.count(),
        record.failedAttempts);
    }

    if (expired)
      _codes.erase(codeIter);
  }

  return authorized;
}

void OtpSystem::PurgeExpired()
{
  const auto now = std::chrono::steady_clock::now();
  const auto& settings = _serverInstance.GetSettings().otp;

  if (now - _lastPurge < settings.purgeInterval)
    return;

  _lastPurge = now;

  // Purge expired codes.
  for (auto it = _codes.begin(); it != _codes.end();)
  {
    if (now > it->second.expiry)
      it = _codes.erase(it);
    else
      ++it;
  }

  // Purge stale lockout records whose lockout has expired
  // and whose failure count is below the threshold (i.e. old records).
  for (auto it = _attempts.begin(); it != _attempts.end();)
  {
    const auto& record = it->second;
    if (record.failedAttempts >= settings.maxFailedAttempts && now > record.lockoutExpiry)
      it = _attempts.erase(it);
    else if (record.failedAttempts < settings.maxFailedAttempts && now > record.lockoutExpiry)
      it = _attempts.erase(it);
    else
      ++it;
  }
}

} // namespace server
