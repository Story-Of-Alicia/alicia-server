//
// Created by rgnter on 19/06/2025.
//

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#ifdef _DEBUG
#define SERVER_DEVELOPMENT_BUILD true
#endif
#ifndef _DEBUG
#define SERVER_DEVELOPMENT_BUILD false
#endif

namespace server
{

namespace constants
{

constexpr bool IsDevelopmentMode = SERVER_DEVELOPMENT_BUILD;
constexpr bool DebugCommands = SERVER_DEVELOPMENT_BUILD;

} // namespace constants

} // namespace server

#undef SERVER_DEVELOPMENT_BUILD

#endif //CONSTANTS_HPP
