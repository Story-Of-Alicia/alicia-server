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
  bool AuthorizeCode(size_t key, uint32_t code);

  uint32_t GrantLtk(size_t key, uint32_t endpointAddress);
  bool AuthorizeLtk(size_t key, uint32_t code, uint32_t endpointAddress);

private:
  struct Code
  {
    std::chrono::steady_clock::time_point expiry{};
    uint32_t code{};
  };

  struct Ltk
  {
    uint32_t code{};
    uint32_t endpointAddress{};
  };

  std::random_device _rd;

  //! Time-based codes.
  std::mutex _codesMutex;
  std::unordered_map<size_t, Code> _codes;

  //! Long-term key codes.
  std::mutex _ltksMutex;
  std::unordered_map<size_t, Ltk> _ltks;
};

} // namespace server

#endif //OTPSYSTEM_HPP
