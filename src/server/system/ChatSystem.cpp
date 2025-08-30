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

#include "server/system/ChatSystem.hpp"

#include "server/ServerInstance.hpp"

#include <libserver/util/Util.hpp>

#include <format>

namespace server
{

void CommandManager::RegisterCommand(
  const std::string& literal,
  Handler handler) noexcept
{
  _commands[literal] = handler;
}

std::vector<std::string> CommandManager::HandleCommand(
  const std::string& literal,
  data::Uid characterUid,
  const std::span<const std::string>& arguments) noexcept
{
  const auto commandIter = _commands.find(literal);
  if (commandIter == _commands.cend())
    return {"Unknown command"};

  try
  {
    return commandIter->second(arguments, characterUid);
  }
  catch (const std::exception& x)
  {
    spdlog::error("Exception executing command handler for '{}': {}", literal, x.what());
    return {"Sever error, contact administrators."};
  }
}

ChatSystem::ChatSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
  // online command
  _commandManager.RegisterCommand(
    "online",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      // todo: better way to collect online players information,
      //       maybe some kind of cental statistics collector
      const auto onlineCharacters = _serverInstance.GetRanchDirector().GetOnlineCharacters();

      std::vector<std::string> response;
      response.emplace_back() = std::format(
        "Online ({}):",
        onlineCharacters.size());

      for (const data::Uid& onlineCharacterUid : onlineCharacters)
      {
        const auto onlineCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
          onlineCharacterUid);

        if (not onlineCharacterRecord)
          continue;

        onlineCharacterRecord.Immutable(
          [&response, characterUid](
            const data::Character& character)
          {
            response.emplace_back() = std::format(
              "{}{}",
              character.name(),
              character.uid() == characterUid ? " (you)" : "");
          });
      }

      return response;
    });

  // visit command
  _commandManager.RegisterCommand(
    "visit",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      // todo: temporary command while messenger is not available

      if (arguments.size() < 1)
        return {"Invalid command argument. (//visit <name>)"};

      // The name of the character the client wants to visit.
      const std::string visitingCharacterName = arguments[0];

      auto visitingCharacterUid = data::InvalidUid;
      bool visitingRanchLocked = true;

      const auto onlineCharacters = _serverInstance.GetRanchDirector().GetOnlineCharacters();

      for (const data::Uid onlineCharacterUid : onlineCharacters)
      {
        const auto onlineCharacterRecord = _serverInstance.GetDataDirector().GetCharacters().Get(
          onlineCharacterUid, false);

        if (not onlineCharacterRecord)
          continue;

        onlineCharacterRecord->Immutable(
          [&visitingCharacterUid, &visitingCharacterName, &visitingRanchLocked](
            const data::Character& character)
          {
            if (visitingCharacterName != character.name())
              return;

            visitingCharacterUid = character.uid();
            visitingRanchLocked = character.isRanchLocked();
          });

        if (visitingCharacterUid != data::InvalidUid)
          break;
      }

      if (visitingCharacterUid != data::InvalidUid)
      {
        if (visitingRanchLocked)
          return {
            std::format(
              "This player's ranch is locked.",
              visitingCharacterName)};

        _serverInstance.GetLobbyDirector().UpdateVisitPreference(
          characterUid, visitingCharacterUid);

        return {
          std::format(
            "Next time you enter the portal, you'll visit {}",
            visitingCharacterName)};
      }

      return {
        std::format("Nobody with the name '{}' is online.", visitingCharacterName),
        "Use //online to view online players."};
    });

  // emblem command
  _commandManager.RegisterCommand(
    "emblem",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      // todo: temporary command while emblems are not persisted

      if (arguments.size() < 1)
        return {
          "Invalid command argument.",
          "(//emblem <ID>)"};

      const uint32_t emblemId = std::atoi(arguments[0].c_str());

      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        characterUid);
      characterRecord.Mutable([emblemId](data::Character& character)
      {
        character.appearance.emblemId = emblemId;
      });

      return {std::format("Set your emblem, restart your game.")};
    });

  // horse command
  _commandManager.RegisterCommand(
    "horse",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      // todo: development command, to be removed

      if (arguments.size() < 1)
        return {
          "Invalid command sub-literal.",
          "(//horse <appearance/parts/randomize>)"};

      const auto& subLiteral = arguments[0];
      auto mountUid = data::InvalidUid;

      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        characterUid);

      if (subLiteral == "parts")
      {
        if (arguments.size() < 5)
          return {
            "Invalid command arguments.",
            "(//horse parts <skinId> <faceId> <maneId> <tailId>)"};

        data::Horse::Parts parts{
          .skinTid = static_cast<uint32_t>(std::atoi(arguments[1].c_str())),
          .faceTid = static_cast<uint32_t>(std::atoi(arguments[2].c_str())),
          .maneTid = static_cast<uint32_t>(std::atoi(arguments[3].c_str())),
          .tailTid = static_cast<uint32_t>(std::atoi(arguments[4].c_str()))};

        characterRecord.Immutable([this, &mountUid, &parts](const data::Character& character)
        {
          _serverInstance.GetDataDirector().GetHorses().Get(character.mountUid())->Mutable(
            [&parts](
              data::Horse& horse)
            {
              if (parts.faceTid() != 0)
                horse.parts.faceTid() = parts.faceTid();
              if (parts.maneTid() != 0)
                horse.parts.maneTid() = parts.maneTid();
              if (parts.skinTid() != 0)
                horse.parts.skinTid() = parts.skinTid();
              if (parts.tailTid() != 0)
                horse.parts.tailTid() = parts.tailTid();
            });

          mountUid = character.mountUid();
        });

        // todo: fix the broadcast
        //BroadcastUpdateMountInfoNotify(clientContext.characterUid, clientContext.visitingRancherUid, mountUid);
        return {
          "Parts set!",
          "Restart the client."};
      }

      if (subLiteral == "appearance")
      {
        if (arguments.size() < 6)
          return {
            "Invalid command arguments.",
            "(//horse appearance <scale> <legLength> <legVolume> <bodyLength> <bodyVolume>)"};
        const data::Horse::Appearance appearance{
          .scale = static_cast<uint32_t>(std::atoi(arguments[1].c_str())),
          .legLength = static_cast<uint32_t>(std::atoi(arguments[2].c_str())),
          .legVolume = static_cast<uint32_t>(std::atoi(arguments[3].c_str())),
          .bodyLength = static_cast<uint32_t>(std::atoi(arguments[4].c_str())),
          .bodyVolume = static_cast<uint32_t>(std::atoi(arguments[5].c_str()))};

        characterRecord.Immutable(
          [this, &mountUid, &appearance](
            const data::Character& character)
          {
            _serverInstance.GetDataDirector().GetHorses().Get(character.mountUid())->Mutable(
              [&appearance](
                data::Horse& horse)
              {
                if (appearance.scale() != 0)
                  horse.appearance.scale() = appearance.scale();
                if (appearance.legLength() != 0)
                  horse.appearance.legLength() = appearance.legLength();
                if (appearance.legVolume() != 0)
                  horse.appearance.legVolume() = appearance.legVolume();
                if (appearance.bodyLength() != 0)
                  horse.appearance.bodyLength() = appearance.bodyLength();
                if (appearance.bodyVolume() != 0)
                  horse.appearance.bodyVolume() = appearance.bodyVolume();
              });
              mountUid = character.mountUid();
            });

        // todo: fix the broadcast
        //BroadcastUpdateMountInfoNotify(clientContext.characterUid, clientContext.visitingRancherUid, mountUid);
        return {
          "Appearance set!",
          "Restart the client."};
      }

      if (subLiteral == "randomize")
      {
        characterRecord.Immutable([this, &mountUid](const data::Character& character)
        {
          _serverInstance.GetDataDirector().GetHorses().Get(character.mountUid())->Mutable(
            [](data::Horse& horse)
            {
              HorseRegistry::Get().BuildRandomHorse(horse.parts, horse.appearance);
            });
          mountUid = character.mountUid();
        });

        // todo: fix the broadcast
        return {"Appearance and parts randomized!",
                "Restart the client."};
      }

      return {"Unknown sub-literal"};
    });

  // give command
  _commandManager.RegisterCommand(
    "give",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      if (arguments.size() < 1)
        return {
          "Invalid command sub-literal.",
          " (//give <item/horse>)"};

      const auto& subLiteral = arguments[0];
      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        characterUid);

      if (subLiteral == "item")
      {
        if (arguments.size() < 3)
          return {
            "Invalid command arguments.",
            "(//give item <count> <tid>)"};

        const uint32_t itemCount = std::atoi(arguments[1].c_str());
        const data::Uid createdItemTid = std::atoi(arguments[2].c_str());

        // Create the item.
        auto createdItemUid = data::InvalidUid;
        const auto createdItemRecord = _serverInstance.GetDataDirector().CreateItem();
        createdItemRecord.Mutable([createdItemTid, itemCount, &createdItemUid](data::Item& item)
        {
          item.tid() = createdItemTid;
          item.count() = itemCount;
          item.expiresAt() = data::Clock::now() + std::chrono::days(10);

          createdItemUid = item.uid();
        });

        // Create the stored item.
        auto giftUid = data::InvalidUid;
        const auto storedItem = _serverInstance.GetDataDirector().CreateStorageItem();
        storedItem.Mutable([this, &giftUid, createdItemUid, createdItemTid](data::StorageItem& storedItem)
        {
          storedItem.items().emplace_back(createdItemUid);
          storedItem.sender() = "System";
          storedItem.message() = std::format("Item '{}'", createdItemTid);
          storedItem.created() = data::Clock::now();

          giftUid = storedItem.uid();
        });

        // Add the stored item as a gift.

        characterRecord.Mutable([giftUid](data::Character& character)
        {
          character.gifts().emplace_back(giftUid);
        });

        return {
          "Item stored in your gift storage.",
          "Check your inventory!"};
      }

      return {"Unknown sub-literal"};
    });
}

ChatSystem::~ChatSystem()
{
}

ChatSystem::Verdict ChatSystem::ProcessChatMessage(
  data::Uid characterUid,
  const std::string& message) noexcept
{
  Verdict verdict;

  if (message.starts_with("//"))
  {
    const auto command = util::TokenizeString(
      message.substr(2), ' ');

    verdict.isBroadcast = false;
    verdict.isSystem = true;
    verdict.result = _commandManager.HandleCommand(
      command[0],
      characterUid,
      std::span(command.begin()+1, command.end()));
  }
  else
  {
    // todo: moderation
    verdict.isBroadcast = true;
    verdict.result = {message};
  }

  return verdict;
}

void ChatSystem::Broadcast(
  std::string message)
{
}

} // namespace server
