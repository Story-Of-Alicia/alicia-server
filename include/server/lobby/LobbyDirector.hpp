//
// Created by rgnter on 25/11/2024.
//

#ifndef LOBBYDIRECTOR_HPP
#define LOBBYDIRECTOR_HPP

#include "LoginHandler.hpp"
#include "server/Settings.hpp"

#include "libserver/data/DataDirector.hpp"
#include "libserver/command/CommandServer.hpp"

namespace alicia
{

class LobbyDirector final
{
public:
  //!
  explicit LobbyDirector(
    soa::DataDirector& dataDirector,
    Settings::LobbySettings settings = {});

  LobbyDirector(const LobbyDirector&) = delete;
  LobbyDirector& operator=(const LobbyDirector&) = delete;

  LobbyDirector(LobbyDirector&&) = delete;
  LobbyDirector& operator=(LobbyDirector&&) = delete;

  void Tick();
  soa::DataDirector& GetDataDirector();
  Settings::LobbySettings& GetSettings();

private:
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
  CommandServer _server;

  //!
  soa::DataDirector& _dataDirector;
  //!
  LoginHandler _loginHandler;

  //!
  std::unordered_map<ClientId, soa::data::Uid> _clientCharacters;
};

}

#endif //LOBBYDIRECTOR_HPP