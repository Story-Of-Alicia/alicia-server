/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
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

#ifndef CHATTERMESSAGEDEFINITIONS_HPP
#define CHATTERMESSAGEDEFINITIONS_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/chatter/ChatterProtocol.hpp>
#include <libserver/util/Stream.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace server::protocol
{

enum class Status : uint8_t
{
  Hidden = 0,
  Offline = 1,
  Online = 2,
  Away = 3,
  Racing = 4,
  WaitingRoom = 5
};

enum class MailboxFolder : uint8_t
{
  Sent = 1,
  Inbox = 2
};

//! Corresponds with `MessengerErrorStrings`.
//! Error codes can be custom server defined error codes.
//! The client displays these custom codes as "Server Error (code: x)"
enum class ChatterErrorCode : uint32_t
{
  LoginFailed = 1,
  CommandCharacterIsNotClientCharacter = 2,
  CharacterDoesNotExist = 3,
  GuildLoginClientNotAuthenticated = 4,
  GuildLoginCharacterNotGuildMember = 5,
  MailInvalidUid = 6,
  MailDoesNotExistOrNotAvailable = 7,
  MailDoesNotBelongToCharacter = 8,
  MailUnknownMailboxFolder = 9,
  MailListInvalidUid = 10,
  LetterDeleteUnknownMailboxFolder = 11,
  LetterDeleteMailUnavailable = 12,
  LetterDeleteMailDoesNotBelongToCharacter = 13,
  LetterDeleteMailDeleteAfterInsertRaceCondition = 14,
  BuddyAddCharacterDoesNotExist = 15,
  BuddyAddCannotAddSelf = 16,
  BuddyAddUnknownCharacter = 17,
  BuddyDeleteTargetCharacterUnavailable = 18,
  BuddyMoveGroupDoesNotExist = 19,
  BuddyMoveAlreadyInGroup = 20,
  BuddyMoveFriendNotFound = 21,
  GroupRenameGroupDoesNotExist = 22,
  GroupRenameDuplicateName = 23,
  GroupDeleteGroupDoesNotExist = 24,
  GroupDeleteDefaultFriendGroupMissing = 25
};

struct Presence
{
  Status status{protocol::Status::Offline};
  enum class Scene : uint32_t
  {
    Ranch = 0,
    Race = 1
  } scene{};
  //! UID of the scene (ranch, room etc). Depends on `scene`.
  data::Uid sceneUid{};

  static void Write(
    const Presence& presence,
    SinkStream& stream);

  static void Read(
    Presence& presence,
    SourceStream& stream);
};

struct ChatCmdLogin
{
  uint32_t characterUid{};
  std::string name{};
  uint32_t code{};
  uint32_t guildUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLogin;
  }

  static void Write(
    const ChatCmdLogin& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLogin& command,
    SourceStream& stream);
};

struct ChatCmdLoginAckOK
{
  uint32_t member1 = 0;

  struct MailAlarm
  {
    enum class Status : uint32_t
    {
      NoNewMail = 0,
      NewMail = 1
    } status{Status::NoNewMail};
    uint8_t hasMail{};
  } mailAlarm;

  struct Group
  {
    uint32_t uid{};
    std::string name{};
  };
  std::vector<Group> groups;

  struct Friend
  {
    uint32_t uid{};
    uint32_t categoryUid{};
    std::string name{};

    Status status = Status::Offline;

    // 2 - friend request popup
    uint8_t member5{};
    Presence::Scene scene{};
    data::Uid sceneUid{};
  };
  std::vector<Friend> friends;

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckOK;
  }

  static void Write(
    const ChatCmdLoginAckOK& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLoginAckOK& command,
    SourceStream& stream);
};

struct ChatCmdLoginAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckCancel;
  }

  static void Write(
    const ChatCmdLoginAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLoginAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdBuddyAdd
{
  std::string characterName{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyAdd;
  }

  static void Write(
    const ChatCmdBuddyAdd& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyAdd& command,
    SourceStream& stream);
};

struct ChatCmdBuddyAddAckOk
{
  //! The uid of the character that is part of the friend request.
  uint32_t characterUid{};
  //! The name of the character that is part of the friend request.
  std::string characterName{};
  //! Unused. This field is the same `ChatCmdLoginAckOK::Friend::member5`.
  uint8_t unk2{};
  //! Online status of the character.
  protocol::Status status{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyAddAckOk;
  }

  static void Write(
    const ChatCmdBuddyAddAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyAddAckOk& command,
    SourceStream& stream);
};

struct ChatCmdBuddyAddAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyAddAckCancel;
  }

  static void Write(
    const ChatCmdBuddyAddAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyAddAckCancel& command,
    SourceStream& stream);
};

//! Clientbound command notifying that a character wants to add them as a friend.
struct ChatCmdBuddyAddRequestTrs
{
  // The uid of the requesting character.
  // Note: `ChatCmdBuddyAddReply` uses this in the reply.
  data::Uid requestingCharacterUid{};
  //! The name of the character requesting to add as friend.
  std::string requestingCharacterName{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyAddRequestTrs;
  }

  static void Write(
    const ChatCmdBuddyAddRequestTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyAddRequestTrs& command,
    SourceStream& stream);
};

//! Serverbound command containing the response by the character for a friend request.
struct ChatCmdBuddyAddReply
{
  //! The uid of the requesting character.
  data::Uid requestingCharacterUid{};
  //! Indicates whether the replying character has accepted the friend request.
  bool requestAccepted{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyAddReply;
  }

  static void Write(
    const ChatCmdBuddyAddReply& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyAddReply& command,
    SourceStream& stream);
};

//! Serverbound command requesting unfriending of a character.
struct ChatCmdBuddyDelete
{
  //! The uid of the character to unfriend.
  data::Uid characterUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyDelete;
  }

  static void Write(
    const ChatCmdBuddyDelete& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyDelete& command,
    SourceStream& stream);
};

//! Clientbound command confirming the unfriending of a character.
struct ChatCmdBuddyDeleteAckOk
{
  //! The uid of the character that is unfriended.
  data::Uid characterUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyDeleteAckOk;
  }

  static void Write(
    const ChatCmdBuddyDeleteAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyDeleteAckOk& command,
    SourceStream& stream);
};

//! Clientbound command cancelling the unfriending of a character.
struct ChatCmdBuddyDeleteAckCancel
{
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyDeleteAckCancel;
  }

  static void Write(
    const ChatCmdBuddyDeleteAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyDeleteAckCancel& command,
    SourceStream& stream);
};

//! Serverbound command requesting a character be moved from one group to another.
struct ChatCmdBuddyMove
{
  //! The uid of the character to move to the selected group.
  data::Uid characterUid{};
  //! The uid of the group the character is being moved to.
  data::Uid groupUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyMove;
  }

  static void Write(
    const ChatCmdBuddyMove& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyMove& command,
    SourceStream& stream);
};

//! Clientbound command acknowledging the moving of a character from one group to another.
struct ChatCmdBuddyMoveAckOk : ChatCmdBuddyMove
{
  // Protocol mirror of `ChatCmdBuddyMove`

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyMoveAckOk;
  }

  static void Write(
    const ChatCmdBuddyMoveAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyMoveAckOk& command,
    SourceStream& stream);
};

struct ChatCmdBuddyMoveAckCancel
{
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdBuddyMoveAckCancel;
  }

  static void Write(
    const ChatCmdBuddyMoveAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdBuddyMoveAckCancel& command,
    SourceStream& stream);
};

//! Serverbound command requesting the creation of a new group.
struct ChatCmdGroupAdd
{
  //! Name of the new group.
  std::string groupName{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupAdd;
  }

  static void Write(
    const ChatCmdGroupAdd& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupAdd& command,
    SourceStream& stream);
};

//! Clientbound command confirming the creation of the group.
struct ChatCmdGroupAddAckOk
{
  //! The uid of the newly created group.
  uint32_t groupUid{};
  //! The name of the newly created group.
  std::string groupName{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupAddAckOk;
  }

  static void Write(
    const ChatCmdGroupAddAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupAddAckOk& command,
    SourceStream& stream);
};

//! Clientbound command indicating an error has occurred when creating a group.
struct ChatCmdGroupAddAckCancel
{
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupAddAckCancel;
  }

  static void Write(
    const ChatCmdGroupAddAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupAddAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdGroupRename
{
  //! The uid of the group to rename.
  data::Uid groupUid{};
  //! The new name to give the group.
  std::string groupName{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupRename;
  }

  static void Write(
    const ChatCmdGroupRename& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupRename& command,
    SourceStream& stream);
};

struct ChatCmdGroupRenameAckOk : ChatCmdGroupRename
{
  // Protocol mirror of `ChatCmdGroupRename`

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupRenameAckOk;
  }

  static void Write(
    const ChatCmdGroupRenameAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupRenameAckOk& command,
    SourceStream& stream);
};

struct ChatCmdGroupRenameAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupRenameAckCancel;
  }

  static void Write(
    const ChatCmdGroupRenameAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupRenameAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdGroupDelete
{
  data::Uid groupUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupDelete;
  }

  static void Write(
    const ChatCmdGroupDelete& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupDelete& command,
    SourceStream& stream);
};

struct ChatCmdGroupDeleteAckOk : ChatCmdGroupDelete
{
  // Protocol mirror of `ChatCmdGroupDelete`

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupDeleteAckOk;
  }

  static void Write(
    const ChatCmdGroupDeleteAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupDeleteAckOk& command,
    SourceStream& stream);
};

struct ChatCmdGroupDeleteAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGroupDeleteAckCancel;
  }

  static void Write(
    const ChatCmdGroupDeleteAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGroupDeleteAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterList
{
  MailboxFolder mailboxFolder{};

  // Likely to do with mailbox pagination
  struct Request
  {
    //! The UID of the last mail in the character mailbox.
    uint32_t lastMailUid{};
    //! Requested mail count to read.
    uint32_t count{};

    static void Write(
      const Request& command,
      SinkStream& stream);

    static void Read(
      Request& command,
      SourceStream& stream);
  } request{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterList;
  }

  static void Write(
    const ChatCmdLetterList& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterList& command,
    SourceStream& stream);
};

struct ChatCmdLetterListAckOk
{
  MailboxFolder mailboxFolder{};

  struct MailboxInfo
  {
    //! Mail count.
    uint32_t mailCount{};
    //! Indicates whether there are more mail in mailbox.
    //! `0` disables the "Show 10 more..." button.
    uint8_t hasMoreMail{};
  } mailboxInfo{};

  struct InboxMail
  {
    //! Mail UID.
    data::Uid uid{};
    data::Mail::MailType type{};
    data::Mail::MailOrigin origin{};

    //! Who sent the mail.
    std::string sender{};
    //! Date of the mail when it was sent, as a string.
    std::string date{};

    struct Struct0
    {
      //! Unknown, left for discovery later.
      std::string unk0{"struct0.unk0"};
      //! Mail body.
      std::string body{};
    } struct0{};
  };
  std::vector<InboxMail> inboxMails{};

  struct SentMail
  {
    data::Uid mailUid{};
    std::string recipient{};
    struct Content
    {
      std::string date{};
      std::string body{};
    } content{};
  };
  std::vector<SentMail> sentMails{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterListAckOk;
  }

  static void Write(
    const ChatCmdLetterListAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterListAckOk& command,
    SourceStream& stream);
};

struct ChatCmdLetterListAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterListAckCancel;
  }

  static void Write(
    const ChatCmdLetterListAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterListAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterSend
{
  std::string recipient{};
  std::string body{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterSend;
  }

  static void Write(
    const ChatCmdLetterSend& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterSend& command,
    SourceStream& stream);
};

struct ChatCmdLetterSendAckOk
{
  // TODO: confirm if this is truly mailUid or relative index of 0 -> x
  data::Uid mailUid{};
  //! Recipient name.
  std::string recipient{};
  //! Client takes anything, typically "hh:mm:ss DD/MM/YYYY".
  std::string date{};
  //! Mail body.
  std::string body{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterSendAckOk;
  }

  static void Write(
    const ChatCmdLetterSendAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterSendAckOk& command,
    SourceStream& stream);
};

struct ChatCmdLetterSendAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterSendAckCancel;
  }

  static void Write(
    const ChatCmdLetterSendAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterSendAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterRead
{
  // Very possibly mailbox folder
  // Typically `2` (inbox?)
  uint8_t unk0{};
  data::Uid mailUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterRead;
  }

  static void Write(
    const ChatCmdLetterRead& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterRead& command,
    SourceStream& stream);
};

struct ChatCmdLetterReadAckOk
{
  //! Very possibly mailbox folder
  // Typically `2` (inbox?)
  uint8_t unk0{};

  //! UID of the mail being requested.
  data::Uid mailUid{};
  std::string unk2{"ChatCmdLetterReadAckOk.unk2"};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterReadAckOk;
  }

  static void Write(
    const ChatCmdLetterReadAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterReadAckOk& command,
    SourceStream& stream);
};

struct ChatCmdLetterReadAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterReadAckCancel;
  }

  static void Write(
    const ChatCmdLetterReadAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterReadAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterDelete
{
  MailboxFolder folder{};
  data::Uid mailUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterDelete;
  }

  static void Write(
    const ChatCmdLetterDelete& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterDelete& command,
    SourceStream& stream);
};

struct ChatCmdLetterDeleteAckOk
{
  MailboxFolder folder{};
  data::Uid mailUid{};
  
  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterDeleteAckOk;
  }

  static void Write(
    const ChatCmdLetterDeleteAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterDeleteAckOk& command,
    SourceStream& stream);
};

struct ChatCmdLetterDeleteAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};
  
  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterDeleteAckCancel;
  }

  static void Write(
    const ChatCmdLetterDeleteAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterDeleteAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterArriveTrs
{
  // Note: almost identical to InboxMail with the extra std::string missing
  data::Uid mailUid{};
  data::Mail::MailType mailType{};
  data::Mail::MailOrigin mailOrigin{};

  std::string sender{};
  std::string date{};
  std::string body{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterArriveTrs;
  }

  static void Write(
    const ChatCmdLetterArriveTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterArriveTrs& command,
    SourceStream& stream);
};

struct ChatCmdUpdateState
{
  Presence presence{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdUpdateState;
  }

  static void Write(
    const ChatCmdUpdateState& command,
    SinkStream& stream);

  static void Read(
    ChatCmdUpdateState& command,
    SourceStream& stream);
};

struct ChatCmdUpdateStateTrs : ChatCmdUpdateState
{
  uint32_t affectedCharacterUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdUpdateStateTrs;
  }

  static void Write(
    const ChatCmdUpdateStateTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdUpdateStateTrs& command,
    SourceStream& stream);
};

struct ChatCmdChatInvite
{
  //! Character UIDs of participants in the chat.
  std::vector<data::Uid> chatParticipantUids{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChatInvite;
  }

  static void Write(
    const ChatCmdChatInvite& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChatInvite& command,
    SourceStream& stream);
};

struct ChatCmdChatInvitationTrs
{
  uint32_t unk0{};
  uint32_t unk1{};
  uint32_t unk2{};
  std::string unk3{};
  uint16_t unk4{};
  uint32_t unk5{};
  
  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChatInvitationTrs;
  }

  static void Write(
    const ChatCmdChatInvitationTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChatInvitationTrs& command,
    SourceStream& stream);
};

struct ChatCmdEnterRoom
{
  uint32_t code{};
  data::Uid characterUid{};
  std::string characterName{};
  data::Uid guildUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdEnterRoom;
  }

  static void Write(
    const ChatCmdEnterRoom& command,
    SinkStream& stream);

  static void Read(
    ChatCmdEnterRoom& command,
    SourceStream& stream);
};

struct ChatCmdEnterRoomAckOk
{
  struct Struct0
  {
    uint32_t unk0{};
    std::string unk1{};
  };
  std::vector<Struct0> unk1{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdEnterRoomAckOk;
  }

  static void Write(
    const ChatCmdEnterRoomAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdEnterRoomAckOk& command,
    SourceStream& stream);
};

struct ChatCmdChat
{
  std::string message{};
  //! Role of the character. `User` and `GameMaster` get sent but `Op` never works.
  enum class Role : uint8_t
  {
    User = 0x0,
    Op = 0x1, // Assumed, tested but no effect
    GameMaster = 0x2
  } role{Role::User};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChat;
  }

  static void Write(
    const ChatCmdChat& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChat& command,
    SourceStream& stream);
};

struct ChatCmdChatTrs
{
  uint32_t unk0{};
  std::string message{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChatTrs;
  }

  static void Write(
    const ChatCmdChatTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChatTrs& command,
    SourceStream& stream);
};

struct ChatCmdInputState
{
  uint8_t state{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdInputState;
  }

  static void Write(
    const ChatCmdInputState& command,
    SinkStream& stream);

  static void Read(
    ChatCmdInputState& command,
    SourceStream& stream);
};

struct ChatCmdInputStateTrs
{
  uint32_t unk0{};
  uint8_t state{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdInputStateTrs;
  }

  static void Write(
    const ChatCmdInputStateTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdInputStateTrs& command,
    SourceStream& stream);
};

struct ChatCmdGameInvite
{
  //! The uid of the character who has been invited.
  data::Uid recipientCharacterUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGameInvite;
  }

  static void Write(
    const ChatCmdGameInvite& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGameInvite& command,
    SourceStream& stream);
};

//! Seems to be not implemented by the client, only deserialised.
struct ChatCmdGameInviteAck
{
  uint32_t unk0{};
  uint32_t unk1{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGameInviteAck;
  }

  static void Write(
    const ChatCmdGameInviteAck& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGameInviteAck& command,
    SourceStream& stream);
};

struct ChatCmdGameInviteTrs
{
  uint32_t unk0{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGameInviteTrs;
  }

  static void Write(
    const ChatCmdGameInviteTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGameInviteTrs& command,
    SourceStream& stream);
};

struct ChatCmdChannelChatTrs
{
  std::string unk0{};

  std::string unk1{};
  uint8_t unk2{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChannelChatTrs;
  }

  static void Write(
    const ChatCmdChannelChatTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChannelChatTrs& command,
    SourceStream& stream);
};

struct ChatCmdChannelInfo
{
  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChannelInfo;
  }

  static void Write(
    const ChatCmdChannelInfo& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChannelInfo& command,
    SourceStream& stream);
};

struct ChatCmdChannelInfoAckOk
{
  std::string hostname{};
  uint16_t port{};
  uint32_t code{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChannelInfoAckOk;
  }

  static void Write(
    const ChatCmdChannelInfoAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChannelInfoAckOk& command,
    SourceStream& stream);
};

struct ChatCmdGuildChannelChatTrs
{
  //! The uid of the destination guild chat.
  data::Uid guildUid{};
  //! Author of the message.
  std::string author{};
  //! Message content.
  std::string message{};
  // TODO: confirm enum values match exactly
  protocol::ChatCmdChat::Role role{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGuildChannelChatTrs;
  }

  static void Write(
    const ChatCmdGuildChannelChatTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGuildChannelChatTrs& command,
    SourceStream& stream);
};

struct ChatCmdGuildLogin : ChatCmdLogin
{
  // ChatCmdGuildLogin shares the same payload as ChatCmdLogin

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGuildLogin;
  }

  static void Write(
    const ChatCmdGuildLogin& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGuildLogin& command,
    SourceStream& stream);
};

struct ChatCmdGuildLoginAckOK
{
  struct GuildMember
  {
    //! Character UID of the guild member. 
    data::Uid characterUid{};
    //! Online presence of the guild member.
    //! Note: `Hidden` completely removes the status of that member.
    Presence presence{};

    static void Write(
      const GuildMember& command,
      SinkStream& stream);

    static void Read(
      GuildMember& command,
      SourceStream& stream);
  };
  std::vector<GuildMember> guildMembers{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGuildLoginAckOK;
  }

  static void Write(
    const ChatCmdGuildLoginAckOK& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGuildLoginAckOK& command,
    SourceStream& stream);
};

struct ChatCmdGuildLoginAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGuildLoginAckCancel;
  }

  static void Write(
    const ChatCmdGuildLoginAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGuildLoginAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdUpdateGuildMemberStateTrs : ChatCmdUpdateStateTrs
{
  // Inherited ChatCmdUpdateStateTrs

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdUpdateGuildMemberStateTrs;
  }

  static void Write(
    const ChatCmdUpdateGuildMemberStateTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdUpdateGuildMemberStateTrs& command,
    SourceStream& stream);
};

struct ChatCmdChannelInfoGuildRoomAckOk : ChatCmdChannelInfoAckOk
{
  // Protocol mirror copy of `ChatCmdChannelInfoAckOk`

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdChannelInfoGuildRoomAckOk;
  }

  static void Write(
    const ChatCmdChannelInfoGuildRoomAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdChannelInfoGuildRoomAckOk& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // CHATTERMESSAGEDEFINITIONS_HPP
