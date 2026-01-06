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

#include "server/messenger/MessengerDirector.hpp"

#include "server/ServerInstance.hpp"

#include <boost/container_hash/hash.hpp>

namespace server
{

//! Alicia client defined friends category.
constexpr auto FriendsCategoryUid = 0;
constexpr auto OnlinePlayersCategoryUid = std::numeric_limits<uint32_t>::max() - 2;
constexpr std::string_view DateTimeFormat = "{:%H:%M:%S %d/%m/%Y} UTC";

MessengerDirector::MessengerDirector(ServerInstance& serverInstance)
  : _chatterServer(*this)
  , _serverInstance(serverInstance)
{
  // Register chatter command handlers
  _chatterServer.RegisterCommandHandler<protocol::ChatCmdLogin>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterLogin(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdBuddyAdd>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterBuddyAdd(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdBuddyAddReply>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterBuddyAddReply(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdBuddyDelete>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterBuddyDelete(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdBuddyMove>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterBuddyMove(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdGroupAdd>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterGroupAdd(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdGroupRename>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterGroupRename(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdGroupDelete>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterGroupDelete(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdLetterList>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterLetterList(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdLetterSend>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterLetterSend(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdLetterRead>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterLetterRead(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdLetterDelete>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterLetterDelete(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdUpdateState>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterUpdateState(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdChatInvite>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterChatInvite(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdChannelInfo>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterChannelInfo(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdGuildLogin>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterGuildLogin(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdGameInvite>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterGameInvite(clientId, command);
    });
}

void MessengerDirector::Initialize()
{
  spdlog::debug(
    "Messenger server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _chatterServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void MessengerDirector::Terminate()
{
  _chatterServer.EndHost();
}

MessengerDirector::ClientContext& MessengerDirector::GetClientContext(
  const network::ClientId clientId,
  bool requireAuthentication)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Messenger client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Messenger client is not authenticated");

  return clientContext;
}

const std::optional<MessengerDirector::Client> MessengerDirector::GetClientByCharacterUid(const data::Uid characterUid) const
{
  std::optional<Client> client{};

  // Get snapshot of current clients
  const auto clientsSnapshot = _clients;
  // Find client iterator by character uid
  const auto& iter = std::ranges::find_if(
    clientsSnapshot,
    [characterUid](const auto& client)
    {
      const ClientContext& clientContext = client.second;
      return clientContext.characterUid == characterUid;
    });

  // If client is found, set client id
  if (iter != clientsSnapshot.cend())
    client.emplace(Client{
      .clientId = iter->first,
      .clientContext = iter->second
    });

  return client;
}

const bool MessengerDirector::IsCharacterOnline(const data::Uid characterUid) const
{
  return GetClientByCharacterUid(characterUid).has_value();
}

void MessengerDirector::Tick()
{
}

Config::Messenger& MessengerDirector::GetConfig()
{
  return _serverInstance.GetSettings().messenger;
}

void MessengerDirector::HandleClientConnected(network::ClientId clientId)
{
  spdlog::debug("Client {} connected to the messenger server from {}",
    clientId,
    _chatterServer.GetClientAddress(clientId).to_string());
  _clients.try_emplace(clientId);
}

void MessengerDirector::HandleClientDisconnected(network::ClientId clientId)
{
  spdlog::debug("Client {} disconnected from the messenger server", clientId);

  // Call update state like a client would do before disconnect
  HandleChatterUpdateState(clientId, protocol::ChatCmdUpdateState{
    .presence = protocol::Presence{
      .status = protocol::Status::Offline,
      .scene = protocol::Presence::Scene::Ranch,
      .sceneUid = 0
    }});

  // TODO: broadcast notify to friends & guilds that character is offline

  _clients.erase(clientId);
}

void MessengerDirector::HandleChatterLogin(
  network::ClientId clientId,
  const protocol::ChatCmdLogin& command)
{
  spdlog::debug("[{}] ChatCmdLogin: {} {} {} {}",
    clientId,
    command.characterUid,
    command.name,
    command.code,
    command.guildUid);

  auto& clientContext = GetClientContext(clientId, false);

  // Generate identity hash based on the character uid from the command and
  // the messenger otp constant
  size_t identityHash = std::hash<uint32_t>()(command.characterUid);
  boost::hash_combine(identityHash, MessengerOtpConstant);

  // Authorise the code received in the command against the calculated identity hash
  clientContext.isAuthenticated = _serverInstance.GetOtpSystem().AuthorizeCode(
    identityHash,
    command.code);

  if (not clientContext.isAuthenticated)
  {
    // Login failed, bad actor, log and return
    // Do not log with `command.name` (character name) to prevent some form of string manipulation in spdlog
    spdlog::warn("Client {} tried to login as character {} but failed authentication with auth code {}",
      clientId,
      command.characterUid,
      command.code);

    protocol::ChatCmdLoginAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::LoginFailed};

    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Store this otp code for reauthentication with the guild login command (if at all)
  clientContext.otpCode = command.code;

  protocol::ChatCmdLoginAckOK response{};

  // TODO: remember status from last login?
  clientContext.presence = protocol::Presence{
    .status = protocol::Status::Online,
    .scene = protocol::Presence::Scene::Ranch,
    .sceneUid = clientContext.characterUid
  };

  // Client request could be logging in as another character
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Mutable(
    [&clientContext](data::Character& character)
    {
      clientContext.characterUid = character.uid();

      // TODO: implement unread mail mechanics

      character.mailbox.hasNewMail() = false;
    });

  response.member1 = clientContext.characterUid;

  // Load friends from character's stored friends list
  std::set<data::Uid> pendingFriends{};
  std::map<data::Uid, data::Character::Contacts::Group> groups{};
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Immutable(
    [&pendingFriends, &groups](const data::Character& character)
    {
      pendingFriends = character.contacts.pending();
      groups = character.contacts.groups();
    });

  // Initialise with one group for now (friends)
  response.groups.emplace_back(FriendsCategoryUid, "");

  // Loop through every group to prepare response
  for (const auto& [groupUid, group] : groups)
  {
    // Add group to response
    response.groups.emplace_back(groupUid, group.name);

    // Build friends list for response
    for (const data::Uid& friendUid : group.members)
    {
      const auto friendCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(friendUid);
      if (!friendCharacterRecord.IsAvailable())
        continue;

      auto& friendo = response.friends.emplace_back();
      friendCharacterRecord.Immutable([&friendo, groupUid](const data::Character& friendCharacter)
      {
        friendo.name = friendCharacter.name();
        friendo.uid = friendCharacter.uid();
        friendo.categoryUid = groupUid;
      });

      // Check if friend is online by looking for them in messenger clients
      friendo.status = protocol::Status::Offline;
      for (const auto& [onlineClientId, onlineClientContext] : _clients)
      {
        if (onlineClientContext.isAuthenticated && onlineClientContext.characterUid == friendUid)
        {
          friendo.status = onlineClientContext.presence.status;
          friendo.scene = onlineClientContext.presence.scene;
          friendo.sceneUid = onlineClientContext.presence.sceneUid;
          break;
        }
      }
    }
  }

  // Pending friend requests
  for (const data::Uid& pendingUid : pendingFriends)
  {
    const auto friendCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(pendingUid);
    if (!friendCharacterRecord.IsAvailable())
      continue;

    auto& friendo = response.friends.emplace_back();
    friendCharacterRecord.Immutable([&friendo](const data::Character& friendCharacter)
    {
      friendo.name = friendCharacter.name();
      friendo.uid = friendCharacter.uid();
      friendo.categoryUid = FriendsCategoryUid;
    });
    
    friendo.member5 = 2; // Pending request
    
    // Check if friend is online by looking for them in messenger clients
    friendo.status = protocol::Status::Offline;
    for (const auto& [onlineClientId, onlineClientContext] : _clients)
    {
      if (onlineClientContext.isAuthenticated && onlineClientContext.characterUid == pendingUid)
      {
        friendo.status = onlineClientContext.presence.status;
        friendo.scene = onlineClientContext.presence.scene;
        friendo.sceneUid = onlineClientContext.presence.sceneUid;
        break;
      }
    }
  }

  _chatterServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // The client sometimes fails to update online state with `ChatCmdUpdateState` command
  // and leaves the server (and subsequently friends/guildmates) in limbo, resulting incorrect online state.
  // Emit online state to relevant players in addition
  // to the client possibly invoking this command.
  HandleChatterUpdateState(clientId, protocol::ChatCmdUpdateState{
    .presence = protocol::Presence{
      .status = protocol::Status::Online,
      .scene = protocol::Presence::Scene::Ranch,
      .sceneUid = clientContext.characterUid // Scene uid is default to character uid by design
    }});
}

void MessengerDirector::HandleChatterBuddyAdd(
  network::ClientId clientId,
  const protocol::ChatCmdBuddyAdd& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Get target character uid by name, if any
  const data::Uid targetCharacterUid = 
    _serverInstance
    .GetDataDirector()
    .GetDataSource()
    .RetrieveCharacterUidByName(command.characterName);

  // Check if character by than name exists
  if (targetCharacterUid == data::InvalidUid)
  {
    // Character by that name does not exist
    // TODO: return protocol::ChatCmdBuddyAddAckCancel (BuddyAddCharacterDoesNotExist)
    return;
  }

  // Get invoker's character name
  // TODO: we could store character name in client context and check instead of retrieving character record
  std::string invokerCharacterName{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&invokerCharacterName](const data::Character& character)
    {
      invokerCharacterName = character.name();
    });

  // Check if invoker is attempting to add itself
  if (command.characterName == invokerCharacterName)
  {
    // Character cannot add itself as a friend.
    // The game should already deny this, but we validate serverside too.
    // TODO: return protocol::ChatCmdBuddyAddAckCancel (BuddyAddCannotAddSelf)
    return;
  }

  // Check if there is already a pending request to the same character
  bool isAlreadyPending{false};
  _serverInstance.GetDataDirector().GetCharacter(targetCharacterUid).Immutable(
    [&isAlreadyPending, requestingCharacterUid = clientContext.characterUid](const data::Character& character)
    {
      isAlreadyPending = std::ranges::contains(
        character.contacts.pending(),
        requestingCharacterUid);
    });

  if (isAlreadyPending)
  {
    // TODO: handle this case (e.g. notify user)
    return;
  }

  // Add to pending friend request
  _serverInstance.GetDataDirector().GetCharacter(targetCharacterUid).Mutable(
    [&clientContext](data::Character& character)
    {
        character.contacts.pending().emplace(clientContext.characterUid);
    });

  // Check if character is online, if so send request live, 
  // else queue it up for when character next comes online.
  const auto clientsSnapshot = _clients;
  auto targetClient = std::ranges::find_if(
    clientsSnapshot,
    [targetCharacterUid](const auto& client)
    {
      return client.second.characterUid == targetCharacterUid;
    });

  // Notify responding character, if they are online
  if (targetClient != clientsSnapshot.cend())
  {
    // Target is online, send friend request to recipient
    const ClientId targetClientId = targetClient->first;
    protocol::ChatCmdBuddyAddRequestTrs notify{
      .requestingCharacterUid = clientContext.characterUid,
      .requestingCharacterName = invokerCharacterName};
    _chatterServer.QueueCommand<decltype(notify)>(targetClientId,[notify](){ return notify; });
  }
}

void MessengerDirector::HandleChatterBuddyAddReply(
  network::ClientId clientId,
  const protocol::ChatCmdBuddyAddReply& command)
{
  spdlog::debug("ChatCmdBuddyAddReply: {} {}",
    command.requestingCharacterUid,
    command.requestAccepted);

  const auto& clientContext = GetClientContext(clientId);
  
  // Get requesting character's record
  const auto& requestingCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
    command.requestingCharacterUid);
  
  // Validate such character exists
  if (not requestingCharacterRecord.IsAvailable())
  {
    // Responding character responded to a friend request from an unknown character uid
    // TODO: return protocol::ChatCmdBuddyAddAckCancel (BuddyAddUnknownCharacter) - will this work?
    return;
  }

  // Get requesting character's name
  std::string requestingCharacterName{};
  requestingCharacterRecord.Immutable(
    [&requestingCharacterName](const data::Character& character)
    {
      requestingCharacterName = character.name();
    });

  // Get responding character's record
  const auto& respondingCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // Get responding character's name
  std::string respondingCharacterName{};
  respondingCharacterRecord.Immutable(
    [&respondingCharacterName](const data::Character& character)
    {
      respondingCharacterName = character.name();
    });

  // If friend request accepted, add each other to friends list
  if (command.requestAccepted)
  {
    // Helper lambda to add a character to character's friends list
    const auto& acceptFriendRequest = [](
      const server::Record<data::Character>& characterRecord,
      const data::Uid characterUid)
    {
      characterRecord.Mutable(
        [characterUid](data::Character& character)
        {
          // Erase other character from character's pending friend requests
          character.contacts.pending().erase(characterUid);
          // Add other character to character's friends list
          auto& groups = character.contacts.groups();
          // Friends group might not be initially initialised, try create it
          auto [friendsGroupIter, created] = groups.try_emplace(FriendsCategoryUid);
          auto& friendsGroup = friendsGroupIter->second;
          if (created)
          {
            // Label the group for internal use only
            // TODO: is this needed? Helps visually but does not affect game
            friendsGroup.name = "_internal_friends_group_";
            friendsGroup.createdAt = util::Clock::now();
          }
          friendsGroup.members.emplace(characterUid);
        });
    };

    // Add responding character to requesting character's friends list
    acceptFriendRequest(requestingCharacterRecord, clientContext.characterUid);

    // Add requesting character to responding character's friends list
    acceptFriendRequest(respondingCharacterRecord, command.requestingCharacterUid);

    // Check if requesting character is online, if so send response live,
    // else simply add responding character to friends list
    const auto clientsSnapshot = _clients;
    auto requestingClient = std::ranges::find_if(
      clientsSnapshot,
      [requestingCharacterUid = command.requestingCharacterUid](const auto& client)
      {
        return client.second.characterUid == requestingCharacterUid;
      });

    protocol::ChatCmdBuddyAddAckOk response{};

    // Keep track of the requesting character's presence (if they are even online)
    std::optional<protocol::Presence> requestingCharacterPresence{};

    // Check if requesting character is still online to notify of friend request result
    if (requestingClient != clientsSnapshot.cend())
    {
      // Requesting character is online
      const ClientId requestingClientId = requestingClient->first;

      const ClientContext& requestingClientContext = requestingClient->second;
      requestingCharacterPresence.emplace(requestingClientContext.presence);

      // Populate response with responding character's information
      response.characterUid = clientContext.characterUid;
      response.characterName = respondingCharacterName;
      response.unk2 = 0; // TODO: identify this
      response.status = clientContext.presence.status;

      // Send response to requesting character
      _chatterServer.QueueCommand<decltype(response)>(requestingClientId, [response](){ return response; });

      // Notify the requesting client of the (invoker) new friend's online state  
      protocol::ChatCmdUpdateStateTrs stateNotify{
        .affectedCharacterUid = clientContext.characterUid};
      stateNotify.presence = clientContext.presence;
      _chatterServer.QueueCommand<decltype(stateNotify)>(requestingClientId, [stateNotify](){ return stateNotify; });
    }

    // Populate response with requesting character's information
    response.characterUid = command.requestingCharacterUid;
    response.characterName = requestingCharacterName;
    response.unk2 = 0; // TODO: identify this

    // Prepare update state for invoker of new friend's online presence
    protocol::ChatCmdUpdateStateTrs stateNotify{
      .affectedCharacterUid = command.requestingCharacterUid};

    if (requestingCharacterPresence.has_value())
    {
      // Requesting character is online, populate fields
      const auto& presence = requestingCharacterPresence.value();
      response.status = presence.status;
      stateNotify.presence = presence;
    }

    // Send response to responding character
    _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });

    // Notify the responding client (invoker) of the requester's (new friend's) online state  
    _chatterServer.QueueCommand<decltype(stateNotify)>(clientId, [stateNotify](){ return stateNotify; });
  }
  else
  {
    // Friend request rejected
    respondingCharacterRecord.Mutable(
      [requestingCharacterUid = command.requestingCharacterUid](data::Character& character)
      {
        character.contacts.pending().erase(requestingCharacterUid);
      });
  }
}

void MessengerDirector::HandleChatterBuddyDelete(
  network::ClientId clientId,
  const protocol::ChatCmdBuddyDelete& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Check if character by that uid even exist
  const auto& targetCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
    command.characterUid);
  if (not targetCharacterRecord.IsAvailable())
  {
    // Character by that uid does not exist or not available
    protocol::ChatCmdBuddyDeleteAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::BuddyDeleteTargetCharacterUnavailable};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Helper lambda to delete a character to character's friends list
  const auto& deleteFriend = [](
    const server::Record<data::Character>& characterRecord,
    const data::Uid characterUid)
  {
    characterRecord.Mutable(
      [characterUid](data::Character& character)
      {
        // Go through all groups and ensure friend is erased
        auto& groups = character.contacts.groups();
        for (auto& [groupUid, group] : groups)
        {
          group.members.erase(characterUid);
        }
      });
  };

  // Delete invoking character from target character's friend list
  deleteFriend(targetCharacterRecord, clientContext.characterUid);

  // Delete target character from invoking character's friend list
  deleteFriend(
    _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid),
    command.characterUid);

  // Return delete confirmation response to invoking character
  protocol::ChatCmdBuddyDeleteAckOk response{
    .characterUid = command.characterUid};
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });

  // Send delete confirmation to target character if they are online
  const auto clientsSnapshot = _clients;
  auto targetClient = std::ranges::find_if(
    clientsSnapshot,
    [targetCharacterUid = command.characterUid](const auto& client)
    {
      return client.second.characterUid == targetCharacterUid;
    });

  // If target character is online then send
  if (targetClient != clientsSnapshot.cend())
  {
    const ClientId targetClientId = targetClient->first;
    // Invoking character's uid to be used for indicating friend delete to target character
    response.characterUid = clientContext.characterUid;
    _chatterServer.QueueCommand<decltype(response)>(targetClientId, [response](){ return response; });
  }
}

void MessengerDirector::HandleChatterBuddyMove(
  network::ClientId clientId,
  const protocol::ChatCmdBuddyMove& command)
{
  spdlog::debug("[{}] ChatCmdBuddyMove: {} {}",
    clientId,
    command.characterUid,
    command.groupUid);

  const auto& clientContext = GetClientContext(clientId);

  // 1. Check if group exists
  // 2. Check if already in that group
  // 3. Check if friends with target character
  // If all good, move friend to group

  std::optional<protocol::ChatterErrorCode> errorCode{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&command, &errorCode](data::Character& character)
    {
      auto& groups = character.contacts.groups();

      // Check if group exists or if friend is in the target group
      if (not groups.contains(command.groupUid))
      {
        // Target group by that uid does not exist
        errorCode.emplace(protocol::ChatterErrorCode::BuddyMoveGroupDoesNotExist);
        return;
      }
      else if (std::ranges::contains(groups.at(command.groupUid).members, command.characterUid))
      {
        // Friend is already in the target group
        errorCode.emplace(protocol::ChatterErrorCode::BuddyMoveAlreadyInGroup);
        return;
      }

      // Go through groups, check if friends with character
      for (auto& [groupUid, group] : character.contacts.groups())
      {
        auto& members = group.members;

        // Find friend in this group
        auto friendIter = std::ranges::find_if(
          members,
          [command](const data::Uid& friendUid)
          {
            return friendUid == command.characterUid;
          });

        if (friendIter != members.cend())
        {
          // Friend found, move friend to the target group
          // Erase friend from current group
          members.erase(friendIter);

          // Add friend to target group
          auto& targetGroup = groups.at(command.groupUid);
          targetGroup.members.emplace(command.characterUid);
          return;
        }
      }

      // Loop did not early return, friend not found
      errorCode.emplace(protocol::ChatterErrorCode::BuddyMoveFriendNotFound);
    });

  if (errorCode.has_value())
  {
    protocol::ChatCmdBuddyMoveAckCancel cancel{
      .errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::ChatCmdBuddyMoveAckOk response{};
  response.characterUid = command.characterUid;
  response.groupUid = command.groupUid;
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterGroupAdd(
  network::ClientId clientId,
  const protocol::ChatCmdGroupAdd& command)
{
  spdlog::debug("[{}] ChatCmdGroupAdd: {}", clientId, command.groupName);

  const auto& clientContext = GetClientContext(clientId);

  // TODO: implement the creation and storing of new group in character
  data::Uid groupUid{data::InvalidUid};
  std::optional<protocol::ChatterErrorCode> errorCode{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&command, &groupUid, &errorCode](data::Character& character)
    {
      auto& groups = character.contacts.groups();
      const auto& nextGroupUid = groups.rbegin()->first + 1;

      // Sanity check if group uid is default friends group uid
      if (nextGroupUid == 0)
      {
        // We have somehow looped back to group uid 0 (which is default friends group)
        // TODO: respond with error code and return;
        return;
      }

      // Create group
      auto [iter, created] = groups.try_emplace(nextGroupUid);
      if (not created)
      {
        // Group by that new group uid already exists
        // Something went terribly wrong with the next group uid logic
        // TODO: respond with error code and return;
        return;
      }

      // Set response group uid
      groupUid = nextGroupUid;

      // Set group information
      auto& group = iter->second;
      group.uid = nextGroupUid;
      group.name = command.groupName;
      group.createdAt = util::Clock::now();
    });
  
  if (errorCode.has_value())
  {
    protocol::ChatCmdGroupAddAckCancel cancel{
      .errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::ChatCmdGroupAddAckOk response{
    .groupUid = groupUid,
    .groupName = command.groupName};
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterGroupRename(
  network::ClientId clientId,
  const protocol::ChatCmdGroupRename& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::optional<protocol::ChatterErrorCode> errorCode{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&command, &errorCode](data::Character& character)
    {
      auto& groups = character.contacts.groups();

      // Check if group exists
      if (not groups.contains(command.groupUid))
      {
        // Group by that uid does not exist
        errorCode.emplace(protocol::ChatterErrorCode::GroupRenameGroupDoesNotExist);
        return;
      }

      // Check if group name is duplicate
      for (const auto& [groupUid, group] : groups)
      {
        if (group.name == command.groupName)
        {
          // Duplicate group name, cancel the rename
          errorCode.emplace(protocol::ChatterErrorCode::GroupRenameDuplicateName);
          return;
        }
      }

      // Set group name
      auto& group = groups.at(command.groupUid);
      group.name = command.groupName;
    });

  if (errorCode.has_value())
  {
    protocol::ChatCmdGroupRenameAckCancel cancel{
      .errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::ChatCmdGroupRenameAckOk response{};
  response.groupUid = command.groupUid;
  response.groupName = command.groupName;
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterGroupDelete(
  network::ClientId clientId,
  const protocol::ChatCmdGroupDelete& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Check if group by that uid exists
  std::optional<protocol::ChatterErrorCode> errorCode{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&command, &errorCode](data::Character& character)
    {
      auto& groups = character.contacts.groups();

      // Confirm the existence of the group and
      // sanity check the existence of default friends group
      if (not groups.contains(command.groupUid))
      {
        // Group does not exist, cannot delete
        errorCode.emplace(protocol::ChatterErrorCode::GroupDeleteGroupDoesNotExist);
        return;
      }
      else if (not groups.contains(FriendsCategoryUid))
      {
        // Default friend group somehow does not exist
        errorCode.emplace(protocol::ChatterErrorCode::GroupDeleteDefaultFriendGroupMissing);
        return;
      }

      // Group to be deleted
      const auto& group = groups.at(command.groupUid);
      // Default friends group
      auto& friendsGroup = groups.at(FriendsCategoryUid);

      // Move all group members back into the friends list
      for (const data::Uid& friendUid : group.members)
      {
        friendsGroup.members.emplace(friendUid);
      }

      // Delete invoked group
      groups.erase(command.groupUid);
    });

  if (errorCode.has_value())
  {
    protocol::ChatCmdGroupDeleteAckCancel cancel{
      .errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::ChatCmdGroupDeleteAckOk response{};
  response.groupUid = command.groupUid;
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterLetterList(
  network::ClientId clientId,
  const protocol::ChatCmdLetterList& command)
{
  bool isInboxRequested = command.mailboxFolder == protocol::MailboxFolder::Inbox;
  bool isSentRequested = command.mailboxFolder == protocol::MailboxFolder::Sent;
  spdlog::debug("[{}] ChatCmdLetterList: {} [{} {}]",
    clientId,
    isInboxRequested ? "Inbox" :
      isSentRequested ? "Sent" : "Unknown",
    command.request.lastMailUid,
    command.request.count);

  if (not isInboxRequested and not isSentRequested)
  {
    spdlog::warn("[{}] ChatCmdLetterList: requested unrecognised mailbox {}",
      clientId,
      static_cast<uint8_t>(command.mailboxFolder));

    protocol::ChatCmdLetterListAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::MailUnknownMailboxFolder};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  const auto& clientContext = GetClientContext(clientId);

  protocol::ChatCmdLetterListAckOk response{
    .mailboxFolder = command.mailboxFolder
  };

  std::string characterName{};
  bool hasMoreMail{false};
  std::vector<data::Uid> mailbox{};

  std::optional<protocol::ChatterErrorCode> errorCode{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&command, &mailbox, &hasMoreMail, &characterName, &errorCode](const data::Character& character)
    {
      characterName = character.name();

      // Get the mailbox based on the command request
      std::vector<data::Uid> _mailbox{};
      if (command.mailboxFolder == protocol::MailboxFolder::Inbox)
        _mailbox = character.mailbox.inbox();
      else if (command.mailboxFolder == protocol::MailboxFolder::Sent)
        _mailbox = character.mailbox.sent();
      else
        throw std::runtime_error("Unrecognised mailbox folder.");

      // Start from the beginning of the mailbox, or from specific mailUid as per request
      auto startIter = _mailbox.begin();
      if (command.request.lastMailUid != data::InvalidUid)
      {
        startIter = std::ranges::find(_mailbox, command.request.lastMailUid);

        // Safety mechanism, just in case no mail by that UID was found
        if (startIter == _mailbox.cend())
        {
          spdlog::warn("Character {} tried to request mail after mail {} but that mail does not exist.",
            character.uid(),
            command.request.lastMailUid);
          hasMoreMail = false;
          errorCode.emplace(protocol::ChatterErrorCode::MailListInvalidUid);
          return;
        }
      }

      // Get remaining items left in the array, from the mailUid (or beginning)
      const auto& remaining = std::distance(
        startIter,
        _mailbox.end());

      // Copy n amounts of mail as per request
      const auto& res = std::ranges::copy_n(
        startIter,
        std::min<size_t>(
          command.request.count,
          remaining),
        std::back_inserter(mailbox));

      // Indicate that there are more mail after the current ending of response mail
      hasMoreMail = res.in != _mailbox.cend();
    });

  if (errorCode.has_value())
  {
    protocol::ChatCmdLetterListAckCancel cancel{
      .errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
  }

  // Track skipped mail to subtract for final response
  uint32_t skippedMailCount{0}; 

  // Build response mailbox
  for (const data::Uid& mailUid : mailbox)
  {
    _serverInstance.GetDataDirector().GetMail(mailUid).Immutable(
      [this, &response, &skippedMailCount, folder = command.mailboxFolder](const data::Mail& mail)
      {
        // Skip soft deleted mails
        if (mail.isDeleted())
        {
          // Increment counter and return
          ++skippedMailCount;
          return;
        }

        // Get mail correspondent depending on the request
        // Mail recipient if sent mailbox or mail sender if inbox mailbox
        data::Uid correspondentUid{data::InvalidUid};
        if (folder == protocol::MailboxFolder::Sent)
          correspondentUid = mail.to();
        else if (folder == protocol::MailboxFolder::Inbox)
          correspondentUid = mail.from();

        // Get correspondent's name to render mail response
        std::string correspondentName{};
        _serverInstance.GetDataDirector().GetCharacter(correspondentUid).Immutable(
          [&correspondentName](const data::Character& character)
          {
            correspondentName = character.name();
          });

        // Format mail createdAt based on format
        const auto& createdAt = std::format(
          DateTimeFormat,
          std::chrono::floor<std::chrono::seconds>(mail.createdAt()));
        if (folder == protocol::MailboxFolder::Sent)
        {
          // Compile sent mail and add to sent mail list
          response.sentMails.emplace_back(
            protocol::ChatCmdLetterListAckOk::SentMail{
              .mailUid = mail.uid(),
              .recipient = correspondentName,
              .content = protocol::ChatCmdLetterListAckOk::SentMail::Content{
                .date = createdAt,
                .body = mail.body()
              }});
        }
        else if (folder == protocol::MailboxFolder::Inbox)
        {
          // Compile sent mail and add to sent mail list
          response.inboxMails.emplace_back(
            protocol::ChatCmdLetterListAckOk::InboxMail{
              .uid = mail.uid(),
              .type = mail.type(),
              .origin = mail.origin(),
              .sender = correspondentName,
              .date = createdAt,
              .struct0 = protocol::ChatCmdLetterListAckOk::InboxMail::Struct0{
                .body = mail.body()
              }
            });
        }
      });
  }

  // `mailbox` size here directly correlates with the loop that processes it 
  // The client is to not be made aware of any skipped mails, adjust mail count
  response.mailboxInfo = protocol::ChatCmdLetterListAckOk::MailboxInfo{
    .mailCount = static_cast<uint32_t>(mailbox.size() - skippedMailCount),
    .hasMoreMail = hasMoreMail
  };

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterLetterSend(
  network::ClientId clientId,
  const protocol::ChatCmdLetterSend& command)
{
  spdlog::debug("[{}] ChatCmdLetterSend: {} [{}]",
    clientId,
    command.recipient,
    command.body);
  
  const data::Uid& recipientCharacterUid = 
    _serverInstance.GetDataDirector().GetDataSource().RetrieveCharacterUidByName(command.recipient);

  if (recipientCharacterUid == data::InvalidUid)
  {
    // Character tried to send mail to a character that doesn't exist, no need to log
    protocol::ChatCmdLetterSendAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::CharacterDoesNotExist
    };

    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // TODO: enforce any character limit?
  // TODO: bad word checks and/or deny sending the letter as a result?

  const auto& clientContext = GetClientContext(clientId);

  std::string senderName{};
  data::Uid senderUid{data::InvalidUid};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&senderUid, &senderName](const data::Character& character)
    {
      senderUid = character.uid();
      senderName = character.name();
    });

  // UTC now in seconds
  const auto& utcNow = std::chrono::floor<std::chrono::seconds>(util::Clock::now());
  const auto& formattedDt = std::format(DateTimeFormat, utcNow);

  // Create and store mail
  data::Uid mailUid{data::InvalidUid};
  auto mailRecord = _serverInstance.GetDataDirector().CreateMail();
  mailRecord.Mutable([&mailUid, &command, &utcNow, &senderUid, &recipientCharacterUid](data::Mail& mail)
  {
    // Set mail parameters
    mail.from() = senderUid;
    mail.to() = recipientCharacterUid;

    mail.type() = data::Mail::MailType::CanReply;
    mail.origin() = data::Mail::MailOrigin::Character;

    mail.createdAt() = utcNow;
    mail.body() = command.body;

    // Get mailUid to store in character record
    mailUid = mail.uid();
  });

  // Add the new mail to the recipient's inbox
  _serverInstance.GetDataDirector().GetCharacter(recipientCharacterUid).Mutable(
    [&mailUid](data::Character& character)
    {
      // Insert new mail to the beginning of the list
      // TODO: this operation is O(n), does dao support std::deque?
      character.mailbox.inbox().insert(
        character.mailbox.inbox().begin(),
        mailUid);

      // Set mail alarm
      character.mailbox.hasNewMail() = true;
    });

  // Add the new mail to the sender's sent mailbox
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&mailUid](data::Character& character)
    {
      // Insert new mail to the beginning of the list
      // TODO: this operation is O(n), does dao support std::deque?
      character.mailbox.sent().insert(
        character.mailbox.sent().begin(),
        mailUid);
    });

  protocol::ChatCmdLetterSendAckOk response{
    .mailUid = mailUid,
    .recipient = command.recipient,
    .date = formattedDt,
    .body = command.body
  };

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });

  // Check if recipient is online for live mail delivery
  auto client = std::ranges::find_if(
    _clients,
    [&recipientCharacterUid](const std::pair<network::ClientId, ClientContext>& client)
    {
      return client.second.characterUid == recipientCharacterUid;
    });

  if (client == _clients.cend())
    // Character is not online, all good and handled
    return;

  protocol::ChatCmdLetterArriveTrs notify{
    .mailUid = mailUid,
    .mailType = data::Mail::MailType::CanReply,
    .mailOrigin = data::Mail::MailOrigin::Character,
    .sender = senderName,
    .date = formattedDt,
    .body = command.body
  };

  const auto& recipientClientId = client->first;
  _chatterServer.QueueCommand<decltype(notify)>(recipientClientId, [notify](){ return notify; });
}

void MessengerDirector::HandleChatterLetterRead(
  network::ClientId clientId,
  const protocol::ChatCmdLetterRead& command)
{
  spdlog::debug("[{}] ChatCmdLetterRead: {} {}",
    clientId,
    command.unk0,
    command.mailUid);

  const auto& clientContext = GetClientContext(clientId);

  // Confirm if the mail even exists
  const auto& mailRecord = _serverInstance.GetDataDirector().GetMail(command.mailUid);

  std::optional<protocol::ChatterErrorCode> errorCode{};
  if (command.mailUid == data::InvalidUid)
  {
    // Character tried to request an invalid mail
    spdlog::warn("Character {} tried to request an invalid mail",
      clientContext.characterUid);
    errorCode.emplace(protocol::ChatterErrorCode::MailInvalidUid);
  }
  else if (not mailRecord.IsAvailable())
  {
    // Mail does not exist or is not available
    spdlog::warn("Character {} tried to request a mail {} that does not exist or is not available",
      clientContext.characterUid,
      command.mailUid);
    errorCode.emplace(protocol::ChatterErrorCode::MailDoesNotExistOrNotAvailable);
  }

  // If we haven't encountered any errors so far, check the mail ownership
  if (not errorCode.has_value())
  {
    // Confirm the character has such mail in its mailbox
    _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
      [&command, &errorCode](const data::Character& character)
      {
        // Check if character has this mail in their `inbox`
        const bool& hasMail = std::ranges::contains(
          character.mailbox.inbox(),
          command.mailUid);

        if (not hasMail)
        {
          // Character does not own this mail
          errorCode.emplace(protocol::ChatterErrorCode::MailDoesNotBelongToCharacter);
        }
      });
  }

  // If an error occurred along the way, respond with cancel and return
  if (errorCode.has_value())
  {
    protocol::ChatCmdLetterReadAckCancel cancel{.errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Mark letter as read
  mailRecord.Mutable([](data::Mail& mail)
  {
    mail.isRead() = true;
  });

  protocol::ChatCmdLetterReadAckOk response{
    .unk0 = command.unk0,
    .mailUid = command.mailUid
  };
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterLetterDelete(
  network::ClientId clientId,
  const protocol::ChatCmdLetterDelete& command)
{
  bool isRequestSent = command.folder == protocol::MailboxFolder::Sent;
  bool isRequestInbox = command.folder == protocol::MailboxFolder::Inbox;
  spdlog::debug("[{}] ChatCmdLetterDelete: {} {}",
    clientId,
    isRequestSent ? "Sent" :
      isRequestInbox ? "Inbox" : "Unknown",
    command.mailUid);

  const auto& clientContext = GetClientContext(clientId);
  
  Record<data::Mail> mailRecord{};
  std::optional<protocol::ChatterErrorCode> errorCode{};
  if (not isRequestSent and not isRequestInbox)
  {
    // Mailbox unrecognised
    spdlog::warn("Character {} tried to delete a mail from unrecognised mailbox {}",
      clientContext.characterUid,
      static_cast<uint8_t>(command.folder));
    errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteUnknownMailboxFolder);
  }
  else if (not (mailRecord = _serverInstance.GetDataDirector().GetMail(command.mailUid)).IsAvailable())
  {
    // Mail is not available
    spdlog::warn("Character {} tried to delete mail {} which is currently not available",
      clientContext.characterUid,
      command.mailUid);
    errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteUnknownMailboxFolder);
  }

  if (not errorCode.has_value())
  {
    // No errors yet, do ownership check
    _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
      [this, &command, &errorCode](data::Character& character)
      {
        switch (command.folder)
        {
          case protocol::MailboxFolder::Sent:
          {
            // Sent mailbox
            // Find mail
            const auto& iter = std::ranges::find(character.mailbox.sent(), command.mailUid);
            // Check if character owns this mail
            if (iter == character.mailbox.sent().cend())
            {
              errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteMailDoesNotBelongToCharacter);
              return;
            }
            break;
          }
          case protocol::MailboxFolder::Inbox:
          {
            // Inbox mailbox
            // Find mail
            const auto& iter = std::ranges::find(character.mailbox.inbox(), command.mailUid);
            // Check if character owns this mail
            if (iter == character.mailbox.inbox().cend())
            {
              errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteMailDoesNotBelongToCharacter);
              return;
            }
            break;
          }
          default:
          {
            throw std::runtime_error(
              std::format(
                "Unrecognised mailbox {}",
                static_cast<uint8_t>(command.folder)));
          }
        }
      });
  }

  if (errorCode.has_value())
  {
    if (errorCode.value() == protocol::ChatterErrorCode::LetterDeleteMailDoesNotBelongToCharacter)
      spdlog::debug("Character {} tried to delete mail {} which they do not own from {} mailbox",
        clientContext.characterUid,
        command.mailUid,
        command.folder == protocol::MailboxFolder::Sent ? "Sent" :
          command.folder == protocol::MailboxFolder::Inbox ? "Inbox" :
          throw std::runtime_error(
            std::format(
              "Unrecognised mailbox {}",
              static_cast<uint8_t>(command.folder))));

    protocol::ChatCmdLetterDeleteAckCancel cancel{.errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Mail exists and character owns this mail, soft delete
  _serverInstance.GetDataDirector().GetMail(command.mailUid).Mutable(
    [](data::Mail& mail)
    {
      mail.isDeleted() = true;
    });
  
  protocol::ChatCmdLetterDeleteAckOk response{
    .folder = command.folder,
    .mailUid = command.mailUid
  };
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterUpdateState(
  network::ClientId clientId,
  const protocol::ChatCmdUpdateState& command)
{
  std::string status =
    command.presence.status == protocol::Status::Hidden ? "Hidden" :
    command.presence.status == protocol::Status::Offline ? "Offline" :
    command.presence.status == protocol::Status::Online ? "Online" :
    command.presence.status == protocol::Status::Away ? "Away" :
    command.presence.status == protocol::Status::Racing ? "Racing" :
    command.presence.status == protocol::Status::WaitingRoom ? "Waiting Room" :
    std::format("Unknown status {}", static_cast<uint8_t>(command.presence.status));

  std::string scene =
    command.presence.scene == protocol::Presence::Scene::Ranch ? "Ranch" :
    command.presence.scene == protocol::Presence::Scene::Race ? "Race" :
    std::format("Unknown scene {}", static_cast<uint32_t>(command.presence.scene));
    
  spdlog::debug("[{}] ChatCmdUpdateState: [{}] [{}] {}",
    clientId,
    status,
    scene,
    command.presence.sceneUid);

  // Sometimes ChatCmdUpdateState is received with status value > 5 containing giberish and causes crashes
  if (static_cast<uint8_t>(command.presence.status) > 5)
  {
    spdlog::warn("Client {} sent unrecognised ChatCmdUpdateState::Status {}",
      clientId,
      static_cast<uint8_t>(command.presence.status));
    return;
  }
  else if (command.presence.status == protocol::Status::Hidden)
  {
    // Do not broadcast this at all as it breaks the row containing 
    // the invoker in other characters' friends list
    return;
  }

  auto& clientContext = GetClientContext(clientId);
  // Update state for client context
  clientContext.presence = command.presence;

  // Get guild uid of the invoking character
  data::Uid guildUid{data::InvalidUid};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&guildUid](const data::Character& character)
    {
      guildUid = character.guildUid();
    });

  std::vector<network::ClientId> guildMembersToNotify{};

  // This mechanism goes through all the online clients and checks if the invoker is in their stored friends list.
  std::vector<network::ClientId> friendsToNotify{};

  const auto clientsSnapshot = _clients;
  for (const auto& [onlineClientId, onlineClientContext] : clientsSnapshot)
  {
    // Skip unauthenticated clients
    bool isAuthenticated = onlineClientContext.isAuthenticated;
    if (not isAuthenticated)
      continue;

    // Self broadcast is needed only for guild notification
    bool isSelf = onlineClientId == clientId;
    if (isSelf and guildUid != data::InvalidUid)
    {
      guildMembersToNotify.emplace_back(onlineClientId);
      continue;
    }

    // Check if invoker is in the online client's stored friends list
    bool isFriend = false;
    _serverInstance.GetDataDirector().GetCharacter(onlineClientContext.characterUid).Immutable(
      [&isFriend, &clientContext](const data::Character& character)
      {
        isFriend = std::ranges::any_of(
          character.contacts.groups() | std::views::values,
          [&clientContext](const data::Character::Contacts::Group& group)
          {
            return std::ranges::contains(group.members, clientContext.characterUid);
          });
      });

    if (isFriend)
    {
      friendsToNotify.emplace_back(onlineClientId);
    }

    // Get online character's guild uid
    data::Uid onlineCharacterGuildUid{data::InvalidUid};
    _serverInstance.GetDataDirector().GetCharacter(onlineClientContext.characterUid).Immutable(
      [&onlineCharacterGuildUid](const data::Character& character)
      {
        onlineCharacterGuildUid = character.guildUid();
      });

    bool isInvokerInAGuild = guildUid != data::InvalidUid;
    bool isOnlineCharacterInAGuild = onlineCharacterGuildUid != data::InvalidUid;
    bool isInvokerAndOnlineCharacterInSameGuild = guildUid == onlineCharacterGuildUid;

    // If invoker is in a guild and other client is in the same guild
    if (isInvokerInAGuild and isOnlineCharacterInAGuild and isInvokerAndOnlineCharacterInSameGuild)
    {
      guildMembersToNotify.emplace_back(onlineClientId);
    } 
  }

  if (not friendsToNotify.empty())
  {
    protocol::ChatCmdUpdateStateTrs notify{
      .affectedCharacterUid = clientContext.characterUid};
    notify.presence = command.presence;

    for (const auto& targetClientId : friendsToNotify)
    {
      _chatterServer.QueueCommand<decltype(notify)>(targetClientId, [notify](){ return notify; });
    }
  }

  if (not guildMembersToNotify.empty())
  {
    protocol::ChatCmdUpdateGuildMemberStateTrs notify{};
    notify.affectedCharacterUid = clientContext.characterUid;
    notify.presence = command.presence;

    for (const auto& targetClientId : guildMembersToNotify)
    {
      _chatterServer.QueueCommand<decltype(notify)>(targetClientId, [notify](){ return notify; });
    }
  }
}

void MessengerDirector::HandleChatterChatInvite(
  network::ClientId clientId,
  const protocol::ChatCmdChatInvite& command)
{
  const auto& clientContext = GetClientContext(clientId);

  constexpr auto concatParticipants =
    [](const std::vector<data::Uid> list, std::string separator = ", ")
    {
      std::string str{};
      for (auto i = 0; i < list.size(); ++i)
      {
        str += std::to_string(list[i]);
        if (i + 1 < list.size())
          str += separator;
      }
      return str;
    };

  spdlog::debug("[{}] ChatCmdChatInvite: [{}]",
    clientId,
    concatParticipants(command.chatParticipantUids));

  std::vector<network::ClientId> clientIdsToNotify{};
  for (const auto& [targetClientId, targetClientContext] : _clients)
  {
    // Skip unauthenticated clients
    if (not targetClientContext.isAuthenticated)
      continue;
    
    bool isRequestedParticipant = std::ranges::contains(
      command.chatParticipantUids,
      targetClientContext.characterUid);
    if (isRequestedParticipant)
    {
      clientIdsToNotify.emplace_back(targetClientId);
      continue;
    }
  }

  if (clientIdsToNotify.empty())
  {
    // No characters by that UID found
    // TODO: ignore request? is there a cancel?
    return;
  }
  
  // TODO: Sent notify to invoker

  protocol::ChatCmdChatInvitationTrs notify{
    .unk0 = 0,
    //.unk1 = 131,
    .unk2 = clientContext.characterUid,
    .unk3 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
    .unk4 = 0,
    .unk5 = clientContext.characterUid};
  for (const auto& targetClientId : clientIdsToNotify)
  {
    const auto& targetClientContext = GetClientContext(targetClientId);
    notify.unk1 = targetClientContext.characterUid;
    _chatterServer.QueueCommand<decltype(notify)>(targetClientId, [notify](){ return notify; });
  }
}

void MessengerDirector::HandleChatterGameInvite(
  network::ClientId clientId,
  const protocol::ChatCmdGameInvite& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Get client id of recipient by character uid
  const std::optional<Client> recipientClient = GetClientByCharacterUid(command.recipientCharacterUid);
  if (not recipientClient.has_value())
  {
    // Character by that uid is not online, cancel
    // TODO: handle this, there's no cancel, ack maybe does something?
    return;
  }

  // TODO: do we need a friendship check?

  // Send game invite notify
  protocol::ChatCmdGameInviteTrs notify{
    .unk0 = clientContext.characterUid};

  const ClientId recipientClientId = recipientClient.value().clientId;
  _chatterServer.QueueCommand<decltype(notify)>(recipientClientId, [notify](){ return notify; });

  protocol::ChatCmdGameInviteAck response{
    .unk0 = clientContext.characterUid,
    .unk1 = command.recipientCharacterUid};
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterChannelInfo(
  network::ClientId clientId,
  const protocol::ChatCmdChannelInfo&)
{
  spdlog::debug("[{}] ChatCmdChannelInfo", clientId);

  const auto& clientContext = GetClientContext(clientId);

  // Get lobby config to get the chat advertisement address and port
  const auto& lobbyConfig = _serverInstance.GetLobbyDirector().GetConfig();

  // Get chat config and check if chat is enabled
  const auto& chatConfig = _serverInstance.GetChatDirector().GetConfig();
  
  if (not chatConfig.enabled)
  {
    // Chat server is disabled
    // TODO: discover (if any) corresponding cancel response exists in game client
    // to get rid of the All/Guild chat tabs
    return;
  }

  // Hash character uid with chat director's otp constant for a unique key
  size_t identityHash = std::hash<uint32_t>()(clientContext.characterUid);
  boost::hash_combine(identityHash, ChatOtpConstant);
  const uint32_t code = _serverInstance.GetOtpSystem().GrantCode(identityHash);

  // Send response for all chat
  protocol::ChatCmdChannelInfoAckOk response{
    .hostname = lobbyConfig.advertisement.chat.address.to_string(),
    .port = lobbyConfig.advertisement.chat.port,
    .code = code};
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });

  bool isInGuild{false};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&isInGuild](const data::Character& character)
    {
      isInGuild = character.guildUid() != 0;
    });

  // If not in a guild, then we're done handling the command
  if (not isInGuild)
    return;

  // Not sending this internally disables the guild chat on the client,
  // even if the client says that guild chat is connected

  // Send response for guild chat
  protocol::ChatCmdChannelInfoGuildRoomAckOk guildResponse{};
  guildResponse.hostname = lobbyConfig.advertisement.chat.address.to_string();
  guildResponse.port = lobbyConfig.advertisement.chat.port;
  guildResponse.code = code; // This value seemingly has no effect
  _chatterServer.QueueCommand<decltype(guildResponse)>(clientId, [guildResponse](){ return guildResponse; });
}

void MessengerDirector::HandleChatterGuildLogin(
  network::ClientId clientId,
  const protocol::ChatCmdGuildLogin& command)
{
  // ChatCmdGuildLogin is sent after ChatCmdLogin

  spdlog::debug("[{}] ChatCmdGuildLogin: {} {} {} {}",
    clientId,
    command.characterUid,
    command.name,
    command.code,
    command.guildUid);

  auto& clientContext = GetClientContext(clientId);

  // Reauthenticate against the already-used otp code that the client
  // gave when authenticating with the `ChatCmdLogin` command handler.
  // The client reuses the otp code that was previously used in `ChatCmdLogin`.
  clientContext.isAuthenticated = command.code == clientContext.otpCode;
  if (not clientContext.isAuthenticated)
  {
    // Login failed, bad actor, log and return
    // Do not log with `command.name` (character name) to prevent some form of string manipulation in spdlog
    spdlog::warn("Client '{}' tried to login to guild '{}' as character '{}' but failed authentication with auth code '{}'",
      clientId,
      command.guildUid,
      command.characterUid,
      command.code);

    protocol::ChatCmdGuildLoginAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::LoginFailed};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Check if client belongs to the guild in the command
  data::Uid characterGuildUid{data::InvalidUid};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&characterGuildUid](const data::Character& character)
    {
      characterGuildUid = character.guildUid();
    });

  std::optional<protocol::ChatterErrorCode> errorCode{};
  if (not clientContext.isAuthenticated)
  {
    // Client is not authenticated with chatter server
    spdlog::warn("Client {} tried to login to guild {} but is not authenticated with the chatter server.",
      clientId,
      command.guildUid);
    errorCode.emplace(protocol::ChatterErrorCode::GuildLoginClientNotAuthenticated);
  }
  else if (command.characterUid != clientContext.characterUid)
  {
    // Command `characterUid` does match the client context `characterUid 
    spdlog::warn("Client {} tried to login, who is character {}, to guild {} on behalf of another character {}",
      clientId,
      clientContext.characterUid,
      command.guildUid,
      command.characterUid);
    errorCode.emplace(protocol::ChatterErrorCode::CommandCharacterIsNotClientCharacter);
  }
  else if (characterGuildUid != command.guildUid)
  {
    // Character does not belong to the guild in the guild login
    spdlog::warn("Character {} tried to login to guild {} but character is not a guild member.",
      clientContext.characterUid,
      command.guildUid);
    errorCode.emplace(protocol::ChatterErrorCode::GuildLoginCharacterNotGuildMember);
  }

  if (errorCode.has_value())
  {
    // Some error has been encountered, respond with cancel and return
    protocol::ChatCmdGuildLoginAckCancel cancel{
      .errorCode = errorCode.value()
    };
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::ChatCmdGuildLoginAckOK response{};
  _serverInstance.GetDataDirector().GetGuild(command.guildUid).Immutable(
    [this, &response](const data::Guild& guild)
    {
      for (const data::Uid& guildMemberUid : guild.members())
      {
        // Create a guild member for the response
        auto& chatGuildMember = response.guildMembers.emplace_back(
          protocol::ChatCmdGuildLoginAckOK::GuildMember{
            .characterUid = guildMemberUid});

        // Find if the guild member is connected to the messenger server
        const auto clientsSnapshot = _clients;
        for (auto& onlineClientContext : clientsSnapshot | std::views::values)
        {
          // If guild member is connected, set status to the one set by the character
          if (onlineClientContext.characterUid == guildMemberUid)
          {
            chatGuildMember.presence = onlineClientContext.presence;
            break;
          }
        }
      }
    });

  _chatterServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Broadcast invoker's online
  HandleChatterUpdateState(
    clientId,
    protocol::ChatCmdUpdateState{
      .presence = clientContext.presence});
}

} // namespace server
