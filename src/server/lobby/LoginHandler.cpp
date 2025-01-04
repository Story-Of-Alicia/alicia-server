//
// Created by rgnter on 26/11/2024.
//

#include "server/lobby/LoginHandler.hpp"

#include <pqxx/pqxx>

namespace alicia
{

LoginHandler::LoginHandler(DataDirector& dataDirector)
    : _dataDirector(_dataDirector)
{
}

bool LoginHandler::Authenticate(const std::string& userName, const std::string& providedUserToken)
{
  // Both strings must be valid.
  if (userName.empty() || providedUserToken.empty())
  {
    return false;
  }

  DatumUid userUid = InvalidDatumUid;
  std::string validUserToken;

  try
  {
    std::scoped_lock connectionLock(_connectionMtx);

    pqxx::transaction tx(_connection);
    const auto result = tx.exec(
      pqxx::prepped(QueryUserStatementId),
      userName);

    tx.commit();

    if (result.size() <= 0)
    {
      return false;
    }

    // extract the data
    std::tie(validUserToken, userUid) = result.one_row().as<
      std::string, DatumUid>();
  }
  catch (std::exception& x)
  {
    spdlog::error("Database issue: {}", x.what());
    return false;
  }

  if (userUid == InvalidDatumUid
    || validUserToken.empty())
  {
    spdlog::warn("Invalid data retrieved from the database for user '{}'", userName);
    assert(false);
    return false;
  }

  return providedUserToken == validUserToken;
}

}


