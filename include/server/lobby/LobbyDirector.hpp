//
// Created by rgnter on 25/11/2024.
//

#ifndef LOBBYDIRECTOR_HPP
#define LOBBYDIRECTOR_HPP

#include "LoginHandler.hpp"
#include "server/DataDirector.hpp"
#include "server/Settings.hpp"

#include "libserver/command/CommandServer.hpp"

namespace alicia
{

class LobbyDirector
{
public:
  //!
  LobbyDirector(
    DataDirector& dataDirector,
    Settings::LobbySettings settings = {});

private:
  //!
  void HandleUserLogin(
    ClientId clientId,
    const LobbyCommandLogin& login);

  //!
  void HandleCreateNicknameOK(
    ClientId clientId,
    const LobbyCommandCreateNicknameOK& createNickname);

  //!
  void HandleEnterChannel(
    ClientId clientId,
    const LobbyCommandEnterChannel& enterChannel);

  //!
  void HandleMakeRoom(
    ClientId clientId,
    const LobbyCommandMakeRoom& makeRoom);

  //!
  void HandleHeartbeat(
    ClientId clientId,
    const LobbyCommandHeartbeat& heartbeat);

  //!
  void HandleShowInventory(
    ClientId clientId,
    const LobbyCommandShowInventory& showInventory);

  //!
  void HandleAchievementCompleteList(
    ClientId clientId,
    const LobbyCommandAchievementCompleteList& achievementCompleteList);

  //!
  void HandleRequestLeagueInfo(
    ClientId clientId,
    const LobbyCommandRequestLeagueInfo& requestLeagueInfo);

  //!
  void HandleRequestQuestList(
    ClientId clientId,
    const LobbyCommandRequestQuestList& requestQuestList);

  //!
  void HandleRequestDailyQuestList(
    ClientId clientId,
    const LobbyCommandRequestDailyQuestList& requestQuestList);

  //!
  void HandleRequestSpecialEventList(
    ClientId clientId,
    const LobbyCommandRequestSpecialEventList& requestQuestList);

  //!
  void HandleEnterRanch(
    ClientId clientId,
    const LobbyCommandEnterRanch& requestEnterRanch);

  //!
  void HandleGetMessengerInfo(
    ClientId clientId,
    const LobbyCommandGetMessengerInfo& requestMessengerInfo);

  //!
  void HandleGoodsShopList(
    ClientId clientId,
    const LobbyCommandGoodsShopList& message);

  //!
  void HandleInquiryTreecash(
    ClientId clientId,
    const LobbyCommandInquiryTreecash& message);

  //!
  void HandleGuildPartyList(
    ClientId clientId,
    const LobbyCommandGuildPartyList& message);

  //!
  Settings::LobbySettings _settings;

  //!
  DataDirector& _dataDirector;
  //!
  LoginHandler _loginHandler;

  //!
  CommandServer _server;
  //!
  std::unordered_map<ClientId, DatumUid> _clientCharacters;

  //!
  struct ClientLoginContext
  {
    std::string userName;

    DatumUid characterUid{InvalidDatumUid};
    LobbyCommandLoginOK response{};
  };

  //!
  std::unordered_map<ClientId, ClientLoginContext> _queuedClientLogins;
};

}

#endif //LOBBYDIRECTOR_HPP