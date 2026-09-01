/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024-2026 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#ifndef RANCHNETWORKHANDLER_HPP
#define RANCHNETWORKHANDLER_HPP

#include "server/tracker/RanchTracker.hpp"

#include <libserver/event/Event.hpp>
#include <libserver/network/command/proto/CommonMessageDefinitions.hpp>
#include <libserver/network/command/proto/RanchMessageDefinitions.hpp>
#include <libserver/network/command/CommandDeferrer.hpp>
#include <libserver/network/command/CommandServer.hpp>
#include <libserver/util/Scheduler.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace server
{

class ServerInstance;

//! An event fired when character connects.
using CharacterConnectEvent = Event<data::Uid>;
//! An event fired when character disconnects.
using CharacterDisconnectEvent = Event<data::Uid>;
//! An event fired when character enters a ranch.
using CharacterRanchEnterEvent = Event<data::Uid, data::Uid>;
//! An event fired when character leaves a ranch.
using CharacterRanchLeaveEvent = Event<data::Uid, data::Uid>;

class RanchNetworkHandler final
  : public CommandServer::EventHandlerInterface
{
public:
  //!
  explicit RanchNetworkHandler(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

  std::vector<data::Uid> GetOnlineCharacters();

  void HandleNetworkTick() override;
  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId client) override;

  //!
  void DisconnectCharacter(data::Uid characterUid);

  //!
  void BroadcastSetIntroductionNotify(
    uint32_t characterUid,
    const std::string& introduction);

  //!
  void BroadcastUpdateMountInfoNotify(
    data::Uid characterUid,
    data::Uid rancherUid,
    data::Uid horseUid);

  //! Send a RequestUser notification to a character connected to this director.
  void SendCharacterSummon(
    data::Uid characterUid,
    bool force,
    std::string characterName,
    uint32_t roomUid,
    uint32_t ranchUid) noexcept;

  //! Show popup notification for client indicating a new item in storage, by character UID
  void SendStorageNotification(
    data::Uid characterUid,
    protocol::AcCmdCRRequestStorage::Category category);

  //! Announces that a foal grew up to an adult to the owning client and the
  //! visitors of its ranch.
  //! @param characterUid UID of the owning character.
  //! @param horseUid UID of the horse that grew up.
  void SendFoalGrowUp(
    data::Uid characterUid,
    data::Uid horseUid);

  void BroadcastChangeAgeNotify(
    data::Uid characterUid,
    data::Uid rancherUid,
    protocol::AcCmdCRChangeAge::Age age);

  void BroadcastHideAgeNotify(
    data::Uid characterUid,
    data::Uid rancherUid,
    protocol::AcCmdCRHideAge::Option option);

  void BroadcastUpdateGuildMemberGradeNotify(
    data::Uid guildUid,
    data::Uid characterUid,
    protocol::GuildRole guildRole);

  void SendDailyQuestNotificationToCharacter(
    data::Uid characterUid,
    const protocol::AcCmdRCUpdateDailyQuestNotify& updateNotify);

  void SendGuildInviteDeclined(
    data::Uid characterUid,
    data::Uid inviterCharacterUid,
    std::string inviterCharacterName,
    data::Uid guildUid);

  void SendGuildInviteAccepted(
    data::Uid guildUid,
    data::Uid characterUid,
    const std::string& newMemberCharacterName);

  void AddRanchHorse(
    data::Uid rancherUid,
    data::Uid horseUid);

  ServerInstance& GetServerInstance();

  [[nodiscard]] CharacterConnectEvent& GetCharacterConnectEvent() noexcept;
  [[nodiscard]] CharacterDisconnectEvent& GetCharacterDisconnectEvent() noexcept;
  [[nodiscard]] CharacterRanchEnterEvent& GetCharacterRanchEnterEvent() noexcept;
  [[nodiscard]] CharacterRanchLeaveEvent& GetCharacterRanchLeaveEvent() noexcept;

private:
  struct ClientContext
  {
    //! User name.
    std::string userName;
    //! Whether the client is authenticated.
    bool isAuthenticated{false};
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Unique ID of the owner of the ranch the client is visiting.
    data::Uid visitingRancherUid{data::InvalidUid};

    //! Whether there's a pending breeding failure card waiting to be claimed
    bool hasPendingFailureCard{false};
    //! Current breeding failure card type.
    protocol::BreedingFailureCardType pendingCardType{};
    //! Fee paid for the failed breeding; scales the failure-card reward grade.
    uint32_t pendingFailureCardSpend{0};

    //! Number of times a deferred ranch entry has been retried while waiting
    //! for horse records to load. Capped so the client isn't stuck forever.
    uint32_t enterRanchDeferAttempts{0};

    //! Number of times a deferred breeding attempt has been retried 
    uint32_t tryBreedingDeferAttempts{0};
  };

  struct RanchInstance
  {
    //! A world tracker of the ranch.
    tracker::RanchTracker tracker;
    //! A set of clients connected to the ranch.
    std::unordered_set<ClientId> clients;
  };

  //! Get client context.
  //! @param clientId Id of the client.
  //! @param requireAuthentication Require the client to be authorized.
  //! @returns Client context.
  [[nodiscard]] ClientContext& GetClientContext(ClientId clientId, bool requireAuthentication = true);

  //! Get the client ID by the character's unique ID.
  //! @param characterUid UID of the character.
  //! @returns Client ID.
  [[nodiscard]] ClientId GetClientIdByCharacterUid(data::Uid characterUid);

  //! Get the client context by the character's unique ID.
  //! @param characterUid UID of the character.
  //! @returns Client context.
  [[nodiscard]] ClientContext& GetClientContextByCharacterUid(data::Uid characterUid);

  //! Handles the ranch enter command.
  //! @param clientId ID of the client
  //! @param command Command
  //! @returns True if the command should be deferred and retried (a required
  //!          horse record was not yet available), false otherwise.
  bool HandleEnterRanch(
    ClientId clientId,
    const protocol::AcCmdCREnterRanch& command);

  void HandleRanchLeave(
    ClientId clientId);

  void ReturnHorseToNature(
    data::Uid characterUid,
    data::Uid horseUid,
    std::string userName,
    bool breedingAbandon);

  void HandleChat(
    ClientId clientId,
    const protocol::AcCmdCRRanchChat& command);

  void HandleSnapshot(
    ClientId clientId,
    const protocol::AcCmdCRRanchSnapshot& command);

  void HandleEnterBreedingMarket(
    ClientId clientId,
    const protocol::AcCmdCREnterBreedingMarket& command);

  void HandleSearchStallion(
    ClientId clientId,
    const protocol::AcCmdCRSearchStallion& command);

  void HandleRegisterStallion(
    ClientId clientId,
    const protocol::AcCmdCRRegisterStallion& command);

  void SendRegisterStallionCancel(
    ClientId clientId);

  void HandleUnregisterStallion(
    ClientId clientId,
    const protocol::AcCmdCRUnregisterStallion& command);

  void SendUnregisterStallionCancel(
    ClientId clientId);

  void HandleUnregisterStallionEstimateInfo(
    ClientId clientId,
    const protocol::AcCmdCRUnregisterStallionEstimateInfo& command);

  void HandleCheckStallionCharge(
    ClientId clientId,
    const protocol::AcCmdCRCheckStallionCharge& command);

  //! Handles the breeding attempt command.
  //! @param clientId ID of the client.
  //! @param command Command.
  //! @returns True if the command should be deferred and retried
  bool HandleTryBreeding(
    ClientId clientId,
    const protocol::AcCmdCRTryBreeding& command);

  //! Rolls a breeding bonus based on the stallion's grade.
  //! @param stallionGrade Grade of the stallion.
  //! @returns The rolled bonus, or a default (id 0) bonus if none activated.
  [[nodiscard]] protocol::BreedingBonus RollBreedingBonus(uint32_t stallionGrade);

  //! Calculates the breeding success rate (0-100).
  //! @param stallionGrade Grade of the stallion.
  //! @param stallionBreedingCount Lifetime breeding count of the stallion.
  //! @param bonus Rolled breeding bonus.
  //! @returns Success rate as a percentage capped at 100.
  [[nodiscard]] uint32_t CalculateBreedingSuccessRate(
    uint32_t stallionGrade,
    uint32_t stallionBreedingCount,
    const protocol::BreedingBonus& bonus);

  //! Creates a foal from a successful breeding, spawns it on the ranch and fills
  //! the breeding response.
  //! @param clientId Client that triggered the breeding.
  //! @param clientContext Context of the breeding client (owner and visited ranch).
  //! @param command Breeding command (holds mare/stallion horse UIDs).
  //! @param bonus Rolled breeding bonus.
  //! @param response Response to populate with the foal's details.
  //! @returns UID of the created foal.
  data::Uid CreateBredFoal(
    ClientId clientId,
    const ClientContext& clientContext,
    const protocol::AcCmdCRTryBreeding& command,
    const protocol::BreedingBonus& bonus,
    protocol::RanchCommandTryBreedingOK& response);

  void HandleBreedingAbandon(
    ClientId clientId,
    const protocol::AcCmdCRBreedingAbandon& command);

  //!
  void HandleBreedingWishlist(
    ClientId clientId,
    const protocol::AcCmdCRBreedingWishlist& command);

  //!
  void HandleBreedingFailureCard(
    ClientId clientId,
    const protocol::AcCmdCRBreedingFailureCard& command);

  //!
  void HandleBreedingFailureCardChoose(
    ClientId clientId,
    const protocol::AcCmdCRBreedingFailureCardChoose& command);

  //!
  void HandleCmdAction(
    ClientId clientId,
    const protocol::AcCmdCRRanchCmdAction& command);

  //!
  void HandleRanchStuff(
    ClientId clientId,
    const protocol::RanchCommandRanchStuff& command);

  //!
  void HandleUpdateBusyState(
    ClientId clientId,
    const protocol::RanchCommandUpdateBusyState& command);

  //!
  void HandleUpdateMountNickname(
    ClientId clientId,
    const protocol::AcCmdCRUpdateMountNickname& command);

  void SendUpdateMountNicknameCancel(
    ClientId clientId,
    protocol::HorseNicknameUpdateError reason);

  //!
  void HandleRequestStorage(
    ClientId clientId,
    const protocol::AcCmdCRRequestStorage& command);

  //!
  void HandleGetItemFromStorage(
    ClientId clientId,
    const protocol::AcCmdCRGetItemFromStorage& command);

  //!
  void HandleRequestNpcDressList(
    ClientId clientId,
    const protocol::RanchCommandRequestNpcDressList& requestNpcDressList);

  void HandleWearEquipment(
    ClientId clientId,
    const protocol::AcCmdCRWearEquipment& command);

  void HandleRemoveEquipment(
    ClientId clientId,
    const protocol::AcCmdCRRemoveEquipment& command);

  void HandleCreateGuild(
    ClientId clientId,
    const protocol::RanchCommandCreateGuild& command);

  void HandleRequestGuildInfo(
    ClientId clientId,
    const protocol::RanchCommandRequestGuildInfo& command);

  void HandleWithdrawGuild(
    ClientId clientId,
    const protocol::AcCmdCRWithdrawGuildMember& command);

  void HandleUpdatePet(
    ClientId clientId,
    const protocol::AcCmdCRUpdatePet& command);

  void SendUpdatePetCancel(
    ClientId clientId,
    const protocol::AcCmdRCUpdatePetCancel& command);

  void HandleIncubateEgg(
    ClientId clientId,
    const protocol::AcCmdCRIncubateEgg& command);

  void HandleBoostIncubateInfoList(
    ClientId clientId,
    const protocol::AcCmdCRBoostIncubateInfoList& command);
  
  void HandleBoostIncubateEgg(
    ClientId clientId,
    const protocol::AcCmdCRBoostIncubateEgg& command);

  void HandleRequestPetBirth(
    ClientId clientId,
    const protocol::AcCmdCRRequestPetBirth& command);

  void HandlePetBornResult(
    ClientId clientId,
    const protocol::AcCmdCRPetBornResult& command);

  void HandleUserPetInfos(
    ClientId clientId,
    const protocol::RanchCommandUserPetInfos& command);

  //! Confirm whether item in the shop can be purchased or gifted.
  void HandleConfirmItem(
    ClientId clientId,
    const protocol::AcCmdCRConfirmItem& command);

  //! Confirm whether item set in the shop can be purchased or gifted.
  void HandleConfirmSetItem(
    ClientId clientId,
    const protocol::AcCmdCRConfirmSetItem& command);

  //! Broadcasts an equipment update of the character owned by the client
  //! to the currently connected ranch.
  //! @param clientId ID of the client.
  void BroadcastEquipmentUpdate(
    ClientId clientId);

  bool HandleUseFoodItem(
    data::Uid mountUid,
    data::Uid characterUid,
    data::Tid usedItemTid,
    protocol::AcCmdCRUseItemOK& response);

  bool HandleUseCleanItem(
    data::Uid mountUid,
    data::Uid characterUid,
    data::Tid usedItemTid,
    protocol::AcCmdCRUseItemOK& response);
  
  bool HandleUsePlayItem(
    data::Uid characterUid,
    data::Uid mountUid,
    data::Tid usedItemTid,
    protocol::AcCmdCRUseItem::PlaySuccessLevel successLevel,
    protocol::AcCmdCRUseItemOK& response);

  bool HandleUseCureItem(
    data::Uid characterUid,
    data::Uid mountUid,
    data::Tid usedItemTid,
    protocol::AcCmdCRUseItemOK& response);

  bool HandleUseGrowItem(
    ClientId clientId,
    data::Uid characterUid,
    data::Uid foalUid);

  void SendMountNotifyAdd(
    ClientId clientId,
    data::Uid horseUid);

  void HandleUseItem(
    ClientId clientId,
    const protocol::AcCmdCRUseItem& command);

  void HandleHousingBuild(
    ClientId clientId,
    const protocol::AcCmdCRHousingBuild& command);

  void HandleHousingRepair(
    ClientId clientId,
    const protocol::AcCmdCRHousingRepair& command);

  //! Spends one use of the character's incubator, deleting it once it is
  //! exhausted.
  //! @param character Character, already held mutable by the caller.
  //! @returns The uses left afterwards, or nullopt when there is nothing to
  //!          spend - no incubator built, or one that never runs out.
  std::optional<uint32_t> ConsumeIncubatorUse(data::Character& character);

  void HandleOpCmd(ClientId clientId,
    const protocol::AcCmdCROpCmd& command);

  void HandleRequestLeagueTeamList(ClientId clientId,
    const protocol::RanchCommandRequestLeagueTeamList& command);

  bool HandleMountFamilyTree(ClientId clientId,
    const protocol::AcCmdCRMountFamilyTree& command);

  void HandleRecoverMount(
    ClientId clientId,
    const protocol::AcCmdCRRecoverMount command);

  void HandleCheckStorageItem(
    ClientId clientId,
    const protocol::AcCmdCRCheckStorageItem command);

  void HandleChangeAge(
    ClientId clientId,
    const protocol::AcCmdCRChangeAge command);

  void HandleHideAge(
    ClientId clientId,
    const protocol::AcCmdCRHideAge command);

  void HandleStatusPointApply(
    ClientId clientId,
    const protocol::AcCmdCRStatusPointApply command);

  void HandleChangeSkillCardPreset(
    ClientId clientId,
    const protocol::AcCmdCRChangeSkillCardPreset command);

  void HandleGetGuildMemberList(
    ClientId clientId,
    const protocol::AcCmdCRGuildMemberList& command);

  void HandleRequestGuildMatchInfo(
    ClientId clientId,
    const protocol::AcCmdCRRequestGuildMatchInfo& command);
  
  void HandleUpdateGuildMemberGrade(
    ClientId clientId,
    const protocol::AcCmdCRUpdateGuildMemberGrade& command);

  void HandleInviteToGuild(
    ClientId clientId,
    const protocol::AcCmdCRInviteGuildJoin& command);
    
  void HandleGetEmblemList(
    ClientId clientId,
    const protocol::AcCmdCREmblemList& command);

  void HandleChangeNickname(
    ClientId clientId, 
    const protocol::AcCmdCRChangeNickname& command);

  void HandleUpdateDailyQuest(
    ClientId clientId,
    const protocol::AcCmdCRUpdateDailyQuest& command);

  void HandleRegisterDailyQuestGroup(
    ClientId clientId,
    const protocol::AcCmdCRRegisterDailyQuestGroup& command);

  void HandleRequestDailyQuestReward(
      ClientId clientId,
      const protocol::AcCmdCRRequestDailyQuestReward& command);

  void HandleRegisterQuest(
      ClientId clientId,
      const protocol::AcCmdCRRegisterQuest& command);

  void HandleRequestQuestReward(
    ClientId clientId,
    const protocol::AcCmdCRRequestQuestReward& command);

  void HandleGiveupQuest(
    ClientId clientId,
    const protocol::AcCmdCRGiveupQuest& command);

  void SendChangeNicknameCancel(
    ClientId clientId,
    protocol::ChangeNicknameError reason);

  void HandleBuyOwnItem(
    ClientId clientId,
    const protocol::AcCmdCRBuyOwnItem& command);

  void HandleSendGift(
    ClientId clientId,
    const protocol::AcCmdCRSendGift& command);

  void HandleOpenRandomBox(
    ClientId clientId,
    const protocol::AcCmdCROpenRandomBox& command);

  void HandleUpdateMountInfo(
    ClientId clientId,
    const protocol::AcCmdCRUpdateMountInfo command);

  void HandlePasswordAuth(
    ClientId clientId,
    const protocol::AcCmdCRPasswordAuth command);

  //! Ranch clients can only invite characters in other ranches.
  void HandleInviteUser(
    ClientId clientId,
    const protocol::AcCmdCRInviteUser& command);

  void HandleRequestUser(
    ClientId clientId,
    const protocol::AcCmdCRRequestUser& command);

  void HandleBreedingTakeMoney(
    ClientId clientId,
    const protocol::AcCmdCRBreedingTakeMoney& command);

  void HandleExpandMountSlot(
    ClientId clientId,
    const protocol::AcCmdCRExpandMountSlot& command);

  void HandleBreedingWishlistAdd(
    ClientId clientId,
    const protocol::AcCmdCRBreedingWishlistAdd& command);

  void HandleBreedingWishlistDelete(
    ClientId clientId,
    const protocol::AcCmdCRBreedingWishlistDel& command);

  CharacterConnectEvent _characterConnectEvent;
  CharacterDisconnectEvent _characterDisconnectEvent;
  CharacterRanchEnterEvent _characterRanchEnterEvent;
  CharacterRanchLeaveEvent _characterRanchLeaveEvent;

  //!
  ServerInstance& _serverInstance;
  //!
  CommandServer _commandServer;

  //!
  std::unordered_map<ClientId, ClientContext> _clients;
  //!
  std::unordered_map<data::Uid, RanchInstance> _ranches;

  //! A command deferrer for the `AcCmdCRMountFamilyTree` command.
  CommandDeferrer<protocol::AcCmdCRMountFamilyTree> _mountFamilyTreeDeferrer;

  //! A command deferrer for the `AcCmdCREnterRanch` command.
  CommandDeferrer<protocol::AcCmdCREnterRanch> _enterRanchDeferrer;

  //! A command deferrer for the `AcCmdCRTryBreeding` command.
  CommandDeferrer<protocol::AcCmdCRTryBreeding> _tryBreedingDeferrer;

  //! Drives periodic ranch chores, such as the foal maturity sweep.
  Scheduler _scheduler;
};

} // namespace server

#endif // RANCHNETWORKHANDLER_HPP
