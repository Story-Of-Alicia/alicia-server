//
// Created by rgnter on 17/07/2025.
//

#ifndef OTPSYSTEM_HPP
#define OTPSYSTEM_HPP

#include <chrono>
#include <random>
#include <mutex>
#include <unordered_map>

namespace server
{

class OtpSystem
{
public:
  uint32_t GrantCode(size_t key);
  bool AuthorizeCode(size_t key, uint32_t code, bool consume = true);

private:
  struct Code
  {
    std::chrono::steady_clock::time_point expiry{};
    uint32_t code{};
  };

  std::mutex _codesMutex;
  std::random_device _rd;
  std::unordered_map<size_t, Code> _codes;
};

} // namespace server

#endif //OTPSYSTEM_HPP
