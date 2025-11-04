//
// Created by maros.prejsa on 14/10/2025.
//

#ifndef ALICIA_SERVER_LOBBYNETWORKHANDLER_HPP
#define ALICIA_SERVER_LOBBYNETWORKHANDLER_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/command/CommandServer.hpp>
#include <libserver/network/command/proto/LobbyMessageDefinitions.hpp>

namespace server
{

class ServerInstance;

class LobbyNetworkHandler final
  : public CommandServer::EventHandlerInterface
{
public:
  explicit LobbyNetworkHandler(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();

  void AcceptLogin(
    ClientId clientId,
    bool sendToCharacterCreator = false);
  void RejectLogin(
    ClientId clientId,
    protocol::AcCmdCLLoginCancel::Reason reason);

  void SendCharacterGuildInvitation(
    data::Uid inviteeUid,
    data::Uid guildUid,
    data::Uid inviterUid);

  [[deprecated]] void SetCharacterVisitPreference(
    data::Uid characterUid,
    data::Uid rancherUid);

  void DisconnectCharacter(
    data::Uid characterUid);
  void MuteCharacter(
    data::Uid characterUid,
    data::Clock::time_point expiration);
  void NotifyCharacter(
    data::Uid characterUid,
    const std::string& message);

private:
  struct ClientContext
  {
    //! A flag indicating whether the client is authenticated.
    bool isAuthenticated{false};
    //! A flag indicating whether the client just created a character.
    bool justCreatedCharacter{false};

    //! A time point of the last heartbeat.
    std::chrono::steady_clock::time_point lastHeartbeat{};

    std::string userName{};
    data::Uid characterUid = data::InvalidUid;
    data::Uid rancherVisitPreference = data::InvalidUid;
  };

  protocol::LobbyCommandLoginOK::SystemContent _systemContent{
    .values = {
      // {4, 0},
      // {16, 0},
      // {21, 0},
      // {22, 0},
      // {30, 0}
    }};

  ClientId GetClientIdByUserName(
    const std::string& userName,
    bool requiresAuthorization = true);
  ClientId GetClientIdByCharacterUid(
    data::Uid characterUid,
    bool requiresAuthorization = true);
  ClientContext& GetClientContext(
    ClientId clientId,
    bool requireAuthentication = true);

  void HandleNetworkTick() override;
  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId clientId) override;

  void HandleLogin(
    ClientId clientId,
    const protocol::AcCmdCLLogin& command);

  void SendLoginOK(
    ClientId clientId);

  void SendLoginCancel(
    ClientId clientId,
    protocol::AcCmdCLLoginCancel::Reason command);

  void HandleRoomList(
    ClientId clientId,
    const protocol::AcCmdCLRoomList& command);

  void HandleHeartbeat(
    ClientId clientId);

  void HandleMakeRoom(
    ClientId clientId,
    const protocol::AcCmdCLMakeRoom& command);

  void HandleEnterRoom(
    ClientId clientId,
    const protocol::AcCmdCLEnterRoom& command);

  void HandleLeaveRoom(
    ClientId clientId);

  void HandleEnterChannel(
    ClientId clientId,
    const protocol::AcCmdCLEnterChannel& command);

  void HandleLeaveChannel(
    ClientId clientId,
    const protocol::AcCmdCLLeaveChannel& command);

  void SendCreateNicknameNotify(
    ClientId clientId);

  void HandleCreateNickname(
    ClientId clientId,
    const protocol::AcCmdCLCreateNickname& command);

  void HandleShowInventory(
    ClientId clientId,
    const protocol::AcCmdCLShowInventory& command);

  void HandleUpdateUserSettings(
    ClientId clientId,
    const protocol::AcCmdCLUpdateUserSettings& command);

  void HandleEnterRoomQuick(
    ClientId clientId,
    const protocol::AcCmdCLEnterRoomQuick& command);

  void HandleGoodsShopList(
    ClientId clientId,
    const protocol::AcCmdCLGoodsShopList& command);

  void HandleAchievementCompleteList(
    ClientId clientId,
    const protocol::AcCmdCLAchievementCompleteList& command);

  void HandleRequestPersonalInfo(
    ClientId clientId,
    const protocol::AcCmdCLRequestPersonalInfo& command);

  void HandleEnterRanch(
    ClientId clientId,
    const protocol::AcCmdCLEnterRanch& command);

  void HandleEnterRanchRandomly(
    ClientId clientId,
    const protocol::AcCmdCLEnterRanchRandomly& command);

  void SendEnterRanchOK(
    ClientId clientId,
    data::Uid rancherUid);

  void HandleFeatureCommand(
    ClientId clientId,
    const protocol::AcCmdCLFeatureCommand& command);

  void HandleRequestFestivalResult(
    ClientId clientId,
    const protocol::AcCmdCLRequestFestivalResult& command);

  void HandleSetIntroduction(
    ClientId clientId,
    const protocol::AcCmdCLSetIntroduction& command);

  void HandleGetMessengerInfo(
    ClientId clientId,
    const protocol::AcCmdCLGetMessengerInfo& command);

  void HandleCheckWaitingSeqno(
    ClientId clientId,
    const protocol::AcCmdCLCheckWaitingSeqno& command);

  void SendWaitingSeqno(
    ClientId clientId,
    size_t queuePosition);

  void HandleUpdateSystemContent(
    ClientId clientId,
    const protocol::AcCmdCLUpdateSystemContent& command);

  void HandleEnterRoomQuickStop(
    ClientId clientId,
    const protocol::AcCmdCLEnterRoomQuickStop& command);

  void HandleRequestFestivalPrize(
    ClientId clientId,
    const protocol::AcCmdCLRequestFestivalPrize& command);

  void HandleQueryServerTime(
    ClientId clientId);

  void HandleRequestMountInfo(
    ClientId clientId,
    const protocol::AcCmdCLRequestMountInfo& command);

  void HandleInquiryTreecash(
    ClientId clientId,
    const protocol::AcCmdCLInquiryTreecash& command);

  void HandleAcceptInviteToGuild(
    ClientId clientId,
    const protocol::AcCmdLCInviteGuildJoinOK& command);

  void HandleDeclineInviteToGuild(
    ClientId clientId,
    const protocol::AcCmdLCInviteGuildJoinCancel& command);

  void HandleClientNotify(
    ClientId clientId,
    const protocol::AcCmdClientNotify& command);

  void HandleChangeRanchOption(
    ClientId clientId,
    const protocol::AcCmdCLChangeRanchOption& command);

  void HandleRequestDailyQuestList(
    ClientId clientId,
    const protocol::AcCmdCLRequestDailyQuestList& command);

  void HandleRequestLeagueInfo(
    ClientId clientId,
    const protocol::AcCmdCLRequestLeagueInfo& command);

  // todo: AcCmdCLMakeGuildParty, AcCmdCLGuildPartyList, AcCmdCLEnterGuildParty,
  //       AcCmdCLLeaveGuildParty, AcCmdCLStartGuildPartyMatch, AcCmdCLStopGuildPartyMatch

  void HandleRequestQuestList(
    ClientId clientId,
    const protocol::AcCmdCLRequestQuestList& command);

  // todo: AcCmdCLChangeGuildPartyOptions,

  void HandleRequestSpecialEventList(
    ClientId clientId,
    const protocol::AcCmdCLRequestSpecialEventList& command);

  //! A server instance.
  ServerInstance& _serverInstance;
  //! A command server.
  CommandServer _commandServer;
  //! A map of clients.
  std::unordered_map<ClientId, ClientContext> _clients;
};

} // namespace server

#endif // ALICIA_SERVER_LOBBYNETWORKHANDLER_HPP
