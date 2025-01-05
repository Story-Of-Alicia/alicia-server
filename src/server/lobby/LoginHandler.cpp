//
// Created by rgnter on 26/11/2024.
//

#include "server/lobby/LoginHandler.hpp"

namespace
{

} // anon namespace

namespace alicia
{

LoginHandler::LoginHandler(DataDirector& dataDirector)
    : _dataDirector(_dataDirector)
{
  _userTokens["rgnt"] = "test";
  _userTokens["laith"] = "another test";
}

bool LoginHandler::Authenticate(const std::string& userName, const std::string& userToken)
{
  return true;

  // Both strings must be valid.
  if (userName.empty() || userToken.empty())
  {
    return false;
  }

  // Find token for the specified user.
  // ToDo: Fetch token from data director.
  const auto& userTokenItr = _userTokens.find(userName);
  if (userTokenItr == _userTokens.cend())
  {
    return false;
  }

  const auto& token = userTokenItr->second;
  // The tokens must match.
  return token == userToken;
}

}


