//
// Created by rgnter on 17/07/2025.
//

#include "server/system/OtpSystem.hpp"

namespace server
{

uint32_t OtpSystem::GrantCode(const uint32_t key)
{
  const auto [iter, inserted] = _codes.try_emplace(
    key,
    Code{
      .expiry = std::chrono::steady_clock::now() + std::chrono::seconds(30),
      .code = _rd()});

  return iter->second.code;
}

bool OtpSystem::AuthorizeCode(const uint32_t key, const uint32_t code)
{
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

} // namespace server
