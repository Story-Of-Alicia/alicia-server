//
// Created by rgnter on 26/11/2024.
//

#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP

#include "server/DataDirector.hpp"

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
    const std::string& providedUserToken);

private:
  //!
  DataDirector& _dataDirector;
};

} // namespace alicia

#endif //LOGINHANDLER_HPP
