//
// Created by rgnter on 17/07/2025.
//

#ifndef OTPREGISTRY_HPP
#define OTPREGISTRY_HPP

#include <chrono>
#include <random>
#include <unordered_map>

namespace server
{

class OtpRegistry
{
public:
  uint32_t GrantCode(uint32_t key);
  bool AuthorizeCode(uint32_t key, uint32_t code);

private:
  struct Code
  {
    std::chrono::steady_clock::time_point expiry{};
    uint32_t code{};
  };

  std::random_device _rd;
  std::unordered_map<uint32_t, Code> _codes;
};

} // namespace server

#endif //OTPREGISTRY_HPP
