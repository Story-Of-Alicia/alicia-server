//
// Created by rgnter on 26/11/2024.
//

#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP

#include "server/DataDirector.hpp"

#include <unordered_map>

namespace alicia
{

//! Login handler.
class LoginHandler
{
public:
  explicit LoginHandler(DataDirector& dataDirector);

  //!
  bool Authenticate(
    const std::string& userName,
    const std::string& userToken);

private:
  //!
  DataDirector& _dataDirector;

  //!
  std::unordered_map<std::string, std::string> _userTokens;
};

} // namespace alicia

#endif //LOGINHANDLER_HPP
