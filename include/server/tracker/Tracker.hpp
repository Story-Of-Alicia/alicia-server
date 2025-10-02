//
// Created by rgnter on 28/09/2025.
//

#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <cstdint>

namespace server::tracker
{

//! Object identifier;
using Oid = uint16_t;
//! Invalid object identifier
constexpr Oid InvalidEntityOid = 0;

} // namespace server::tracker

#endif //TRACKER_HPP
