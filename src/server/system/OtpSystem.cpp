//
// Created by rgnter on 17/07/2025.
//

#include "server/system/OtpSystem.hpp"

namespace server
{

uint32_t OtpSystem::GrantCode(const size_t key)
{
  std::scoped_lock lock(_codesMutex);
  const auto [iter, inserted] = _codes.insert_or_assign(
    key,
    Code{
      .expiry = std::chrono::steady_clock::now() + std::chrono::seconds(30),
      .code = _rd()});

  return iter->second.code;
}

bool OtpSystem::AuthorizeCode(const size_t key, const uint32_t code)
{
  std::scoped_lock lock(_codesMutex);

  const auto codeIter = _codes.find(key);
  if (codeIter == _codes.cend())
    return false;

  const Code& ctx = codeIter->second;

  const bool expired = std::chrono::steady_clock::now() > ctx.expiry;
  const bool authorized = not expired && ctx.code == code;
  if (authorized)
    _codes.erase(codeIter);

  return authorized;
}

uint32_t OtpSystem::GrantLtk(size_t key, uint32_t endpointAddress)
{
  std::scoped_lock lock(_ltksMutex);
  const auto [iter, inserted] = _ltks.insert_or_assign(
    key,
    Ltk{
      .code = _rd(),
      .endpointAddress = endpointAddress});

  return iter->second.code;
}

bool OtpSystem::AuthorizeLtk(size_t key, uint32_t code, uint32_t endpointAddress)
{
  std::scoped_lock lock(_ltksMutex);

  const auto ltkIter = _ltks.find(key);
  if (ltkIter == _ltks.cend())
    return false;

  const Ltk& ctx = ltkIter->second;

  const bool isSameEndpointAddress = ctx.endpointAddress == endpointAddress;
  const bool authorized = ctx.code == code and isSameEndpointAddress;

  return authorized;
}

} // namespace server
