//
// Created by rgnter on 17/07/2025.
//

#ifndef OTPSYSTEM_HPP
#define OTPSYSTEM_HPP

#include <chrono>
#include <random>
#include <unordered_map>

namespace server
{

class OtpSystem
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

#endif //OTPSYSTEM_HPP
