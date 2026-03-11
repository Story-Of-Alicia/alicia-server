//
// Created by rgnter on 17/07/2025.
//

#ifndef OTPSYSTEM_HPP
#define OTPSYSTEM_HPP

#include <chrono>
#include <mutex>
#include <random>
#include <unordered_map>

namespace server
{

class OtpSystem
{
public:
  struct Settings
  {
    std::chrono::seconds codeTtl{30};
    uint32_t maxFailedAttempts{5};
    std::chrono::seconds lockoutDuration{60};
    std::chrono::seconds purgeInterval{60};

    Settings() = default;
  };

  OtpSystem();
  explicit OtpSystem(Settings settings);

  //! Grants a one-time code for the given key.
  //! If a code already exists for this key, it is replaced.
  //! @param key Identity key the code is bound to.
  //! @return The generated one-time code.
  [[nodiscard]] uint32_t GrantCode(size_t key);

  //! Authorizes a one-time code for the given key.
  //! If the code is unsuccessfully authorized multiple times the code is invalidated
  //! to protect against brute force.
  //! @param key Identity key the code is bound to.
  //! @param code The code to verify.
  //! @param consume If true, the code is consumed (erased) on success.
  //! @retval `true` if the code is valid
  //! @retval `false` if the code is invalid.
    [[nodiscard]] bool AuthorizeCode(size_t key, uint32_t code, bool consume = true);

  //! Removes all expired codes and stale lockout entries.
  //! Called automatically on a periodic basis, but can be invoked manually.
  void PurgeExpired();

private:
  Settings _settings;

  struct Code
  {
    std::chrono::steady_clock::time_point expiry{};
    uint32_t code{};
  };

  struct AttemptRecord
  {
    uint32_t failedAttempts{0};
    std::chrono::steady_clock::time_point lockoutExpiry{};
  };

  std::mutex _codesMutex;
  std::mt19937 _rng;
  std::unordered_map<size_t, Code> _codes;
  std::unordered_map<size_t, AttemptRecord> _attempts;
  std::chrono::steady_clock::time_point _lastPurge;
};

} // namespace server

#endif // OTPSYSTEM_HPP
