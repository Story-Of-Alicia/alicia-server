//
// Created by rgnter on 17/07/2025.
//

#include "server/system/OtpSystem.hpp"

namespace server
{

uint32_t OtpSystem::GrantCode(const size_t key, bool ltk)
{
  std::scoped_lock lock(_codesMutex);
  Code code{
    .code = _rd(),
    .ltk = ltk};

  if (not ltk)
    code.expiry = std::chrono::steady_clock::now() + std::chrono::seconds(30);

  const auto [iter, inserted] = _codes.insert_or_assign(key, code);
  return iter->second.code;
}

bool OtpSystem::AuthorizeCode(const size_t key, const uint32_t code)
{
  std::scoped_lock lock(_codesMutex);

  const auto codeIter = _codes.find(key);
  if (codeIter == _codes.cend())
    return false;

  const Code& ctx = codeIter->second;

  const bool expired = not ctx.ltk and std::chrono::steady_clock::now() > ctx.expiry;
  const bool authorized = not expired && ctx.code == code;
  if (authorized and not ctx.ltk)
    _codes.erase(codeIter);

  return authorized;
}

} // namespace server
