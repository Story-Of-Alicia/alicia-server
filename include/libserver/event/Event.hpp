//
// Created by maros.prejsa on 20/02/2026.
//

#ifndef ALICIA_SERVER_EVENT_HPP
#define ALICIA_SERVER_EVENT_HPP

#include <functional>

namespace server
{

template<typename... T>
class Event final
{
public:
  using Listener = std::function<void(T...)>;
  using ListenerList = std::list<Listener>;
  using ListenerHandle = ListenerList::iterator;
;
  [[nodiscard]] ListenerHandle Subscribe(Listener& listener) noexcept
  {
    return _listeners.emplace(_listeners.back(), listener);
  }

  void Unsubscribe(ListenerHandle handle) noexcept
  {
    _listeners.erase(handle);
  }

  void Fire(T... payload)
  {
    for (const auto& listener : _listeners)
    {
      listener(payload...);
    }
  }

private:
  ListenerList _listeners;
};

} // server

#endif // ALICIA_SERVER_EVENT_HPP
