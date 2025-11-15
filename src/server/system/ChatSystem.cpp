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
#include "Version.hpp"

#include <libserver/util/Util.hpp>

#include <regex>
#include <format>

namespace server
{

namespace
{

constexpr std::string_view UserLine        = "  - user: '{}'";
constexpr std::string_view CharacterLine   = "    <font color=\"#AAAAAA\">(uid:{}) '{}', level {}</font>";
constexpr std::string_view NoCharacterLine = "    <font color=\"#FF0000\">no character</font>";

} // anon namespace

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
    return {"Server error, contact administrators."};
  }
}

ChatSystem::ChatSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
  RegisterUserCommands();
  RegisterAdminCommands();
}

ChatSystem::~ChatSystem()
{
}

ChatSystem::ChatVerdict ChatSystem::ProcessChatMessage(
  data::Uid characterUid,
  const std::string& message) noexcept
{
  ChatVerdict verdict;

  if (message.starts_with("//"))
  {
    verdict.commandVerdict = ProcessCommandMessage(
      characterUid, message.substr(2));
  }
  else
  {
    // todo: auto moderation
    verdict.message = message;
  }

  return verdict;
}

ChatSystem::CommandVerdict ChatSystem::ProcessCommandMessage(
  data::Uid characterUid,
  const std::string& message)
{
  CommandVerdict verdict;

  const auto command = util::TokenizeString(
    message, ' ');

  verdict.result = _commandManager.HandleCommand(
    command[0],
    characterUid,
    std::span(command.begin() + 1, command.end()));

  return verdict;
}

void ChatSystem::RegisterUserCommands()
{
  // about command
  _commandManager.RegisterCommand(
    "about",
    [this](
      const std::span<const std::string>& arguments,
      [[maybe_unused]] data::Uid characterUid) -> std::vector<std::string>
    {
      const std::string brandName = _serverInstance.GetSettings().general.brand;

      return {
        "Story of Alicia dedicated server software",
        " available under the GPL-2.0 license",
        "",
        "Running story-of-alicia/alicia-server",
        std::format("  v{}", BuildVersion),
        std::format("Hosted by:"),
        std::format("  {}", brandName)};
    });

  // help command
  _commandManager.RegisterCommand(
    "help",
    [this](
      const std::span<const std::string>& arguments,
      [[maybe_unused]] data::Uid characterUid) -> std::vector<std::string>
    {
      return {
        "Command are a subject of the prototype.",
        "For command reference, ask the community ",
        " or browse the code online.",
        " ",
        "Official user command reference:",
        " //about - Information about the server",
        " //create - Sends you to the character creator",
        " //online - Information about players",
        " ",
        "Official admin command reference:",
        " //infraction - Infraction management",
        " //incognito - Toggles incognito mode for GMs"
        " //info - Info about users and characters",
        " //promote - Promotes user to Game Master role",
        " //demote - Demotes user to User role",
        " //notice - Sends notice to character",
        " //users - Overview of users",
        " ",
        "More commands available over at: ",
        " https://bruhvrum.github.io/registertest/commands"};
    });

  // create command
  _commandManager.RegisterCommand(
    "create",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      _serverInstance.GetLobbyDirector().SetCharacterForcedIntoCreator(characterUid, true);
      return {
        "Once you restart your game,",
        " you'll enter the character creator.",
        "You may not change your name there,",
        " please use the dedicated item instead"};
    });

  // online command
  _commandManager.RegisterCommand(
    "online",
    [this](
      [[maybe_unused]] const std::span<const std::string>& arguments,
      [[maybe_unused]] data::Uid characterUid) -> std::vector<std::string>
    {
      std::vector<std::string> response;

      const auto& userInstances = _serverInstance.GetLobbyDirector().GetUsers();

      response.emplace_back() = std::format(
        "Online ({}):",
        userInstances.size());

      for (const auto& userInstance : userInstances | std::views::values)
      {
        const auto userRecord = _serverInstance.GetDataDirector().GetUser(
          userInstance.userName);
        if (userRecord)
        {
          auto onlineCharacterUid{data::InvalidUid};
          userRecord.Immutable([&onlineCharacterUid](const data::User& user)
          {
            onlineCharacterUid = user.characterUid();
          });

          const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
            onlineCharacterUid);
          if (characterRecord)
          {
            characterRecord.Immutable([&response](const data::Character& character)
            {
              response.emplace_back(std::format(" {}", character.name()));
            });
          }
        }
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
        const auto onlineCharacterRecord = _serverInstance.GetDataDirector().GetCharacterCache().Get(
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

        _serverInstance.GetLobbyDirector().SetCharacterVisitPreference(
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
          "(//horse <appearance/parts/potential/randomize>)"};

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
            _serverInstance.GetDataDirector().GetHorseCache().Get(character.mountUid())->Mutable([&parts](data::Horse& horse)
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
        // BroadcastUpdateMountInfoNotify(clientContext.characterUid, clientContext.visitingRancherUid, mountUid);
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
            _serverInstance.GetDataDirector().GetHorseCache().Get(character.mountUid())->Mutable([&appearance](data::Horse& horse)
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
        // BroadcastUpdateMountInfoNotify(clientContext.characterUid, clientContext.visitingRancherUid, mountUid);
        return {
          "Appearance set!",
          "Restart the client."};
      }

      if (subLiteral == "potential")
      {
        if (arguments.size() > 1 && arguments[1] == "random")
        {
          characterRecord.Immutable(
            [this](const data::Character& character)
            {
              _serverInstance.GetDataDirector().GetHorse(character.mountUid()).Mutable(
                [this](data::Horse& horse)
                {
                  _serverInstance.GetHorseRegistry().GiveHorseRandomPotential(
                    horse.potential);
                });
            });
        }
        else if (arguments.size() > 3)
        {
          const auto& type = static_cast<uint8_t>(std::atoi(arguments[1].c_str()));
          const auto& level = static_cast<uint8_t>(std::atoi(arguments[2].c_str()));
          const auto& value = static_cast<uint8_t>(std::atoi(arguments[3].c_str()));

          // Table MountPotentialInfo has all but index 12
          // Valid range: 0 < type < 16
          constexpr uint8_t InvalidPotential = 12;
          if (type == InvalidPotential || type > 15 || type < 1)
            return {"Invalid horse potential type, range 1-15 (except 12)"};

          characterRecord.Immutable(
            [this, &type, &level, &value](
              const data::Character& character)
            {
              _serverInstance.GetDataDirector().GetHorse(character.mountUid()).Mutable(
                [this, &type, &level, &value](data::Horse& horse)
                {
                  _serverInstance.GetHorseRegistry().SetHorsePotential(
                    horse.potential,
                    type,
                    level,
                    value);
                });
            });
        }
        else
        {
          return {
            "Invalid command arguments.",
            "(//horse potential random)",
            "(//horse potential <type> <level> <value>)"};
        }

        return {"Horse potential set",
          "Restart the client."};
      }

      if (subLiteral == "randomize")
      {
        characterRecord.Immutable([this, &mountUid](const data::Character& character)
          {
            _serverInstance.GetDataDirector().GetHorseCache().Get(character.mountUid())->Mutable(
              [this](data::Horse& horse)
              {
                _serverInstance.GetHorseRegistry().BuildRandomHorse(
                  horse.parts, horse.appearance);
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
          " (//give <item/horse/preset>)"};

      const auto& subLiteral = arguments[0];
      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        characterUid);

      if (subLiteral == "item")
      {
        if (arguments.size() < 3)
          return {
            "Invalid command arguments.",
            "(//give item <count> <tid>)"};

        const int32_t itemCount = std::atoi(arguments[1].c_str());
        const data::Uid createdItemTid = std::atoi(arguments[2].c_str());

        if (itemCount < 1)
        {
          return {"Invalid item count"};
        }

        if (createdItemTid >= 99000 && createdItemTid <= 99200)
        {
          return {"Please give yourself eggs to hatch pets."};
        }

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
        storedItem.Mutable([this, &giftUid, itemCount, createdItemUid, createdItemTid](data::StorageItem& storedItem)
          {
            storedItem.items().emplace_back(createdItemUid);
            storedItem.sender() = "System";

            const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(createdItemTid);
            if (itemTemplate)
              storedItem.message() = std::format("{}x Item '{}'", itemCount, itemTemplate->name);
            else
              storedItem.message() = std::format("{}x Item '{}'", itemCount, createdItemTid);

            storedItem.created() = data::Clock::now();

            giftUid = storedItem.uid();
          });

        // Add the stored item as a gift.

        characterRecord.Mutable([giftUid](data::Character& character)
          {
            character.gifts().emplace_back(giftUid);
          });

        _serverInstance.GetRanchDirector().SendStorageNotification(
          characterUid, protocol::AcCmdCRRequestStorage::Category::Gifts);

        return {
          "Item stored in your gift storage.",
          "Check your inventory!"};
      }
      else if (subLiteral == "preset")
      {
        // //give preset <care> [<count>]
        if (arguments.size() < 2)
          return {
            "Invalid command arguments.",
            "(//give preset <all|feed|clean|play|cure|construct> [<count>])"};

        const auto selectedPreset = arguments[1];
        int32_t itemCount = 1;
        // Check if <count> is supplied
        if (arguments.size() > 2)
          itemCount = std::atoi(arguments[2].c_str());

        if (itemCount < 1)
        {
          return {"Invalid item count"};
        }

        std::map<std::string, std::vector<data::Tid>> presets{
          {"feed", {41001, 41002, 41003, 41004, 41005, 41006, 41007}},
          {"clean", {40002, 41008, 41009}},
          {"play", {42001, 42002}},
          {"cure", {44001, 44002, 44003, 44004, 44005, 44006}},
          {"construct", {45001, 46018, 45004, 70002}}};

        std::vector<data::Tid> selectedItems{};
        if (selectedPreset == "all")
        {
          // Add all items from the entire preset range
          for (const auto& preset : presets | std::views::values)
          {
            selectedItems.insert(
              selectedItems.end(),
              preset.begin(),
              preset.end());
          }
        }
        else if (presets.contains(selectedPreset))
        {
          // Pick from presets list
          selectedItems = presets[selectedPreset];
        }
        else
        {
          // Preset not found
          return {"Unknown preset"};
        }

        for (const auto& selectedItemTid : selectedItems)
        {
          // Create the item.
          auto createdItemUid = data::InvalidUid;
          const auto createdItemRecord = _serverInstance.GetDataDirector().CreateItem();
          createdItemRecord.Mutable([selectedItemTid, itemCount, &createdItemUid](data::Item& item)
            {
              item.tid() = selectedItemTid;
              item.count() = itemCount;
              item.expiresAt() = data::Clock::now() + std::chrono::days(10);

              createdItemUid = item.uid();
            });

          // Add the item directly to character's inventory.
          characterRecord.Mutable([createdItemUid](data::Character& character)
            {
              character.inventory().emplace_back(createdItemUid);
            });
        }

        return {"Preset added to character inventory. Please restart your game to apply changes!"};
      }
      else if (subLiteral == "horse")
      {
        // Check if character has max amount of horses
        auto horseCount = 0;
        auto horseSlotCount = 0;
        characterRecord.Immutable(
          [&horseCount, &horseSlotCount](const data::Character& character)
          {
            // Mount + horses (if any)
            horseCount = character.horses().size() + 1;
            horseSlotCount = character.horseSlotCount();
          });

        constexpr uint8_t MaxHorsePerCharacter = 10;
        if (horseCount >= MaxHorsePerCharacter)
          return {"You already have max amount of horses in your inventory."};

        auto horseUid = data::InvalidUid;
        const auto& horseRecord = _serverInstance.GetDataDirector().CreateHorse();
        horseRecord.Mutable(
          [this, &horseUid](data::Horse& horse)
          {
            // Prepare new horse with initial values
            horse.tid() = 20002;
            horse.dateOfBirth() = data::Clock::now();
            horse.mountCondition.stamina = 3500;
            horse.growthPoints() = 150;

            // Give horse random parts and appearance
            _serverInstance.GetHorseRegistry().BuildRandomHorse(
              horse.parts,
              horse.appearance);

            horseUid = horse.uid();
          });

        characterRecord.Mutable(
          [&horseUid](data::Character& character)
          {
            // Increment horse slot count if we can
            if (character.horseSlotCount() <= MaxHorsePerCharacter)
              character.horseSlotCount() += 1;

            // Add new horse to character
            character.horses().emplace_back(horseUid);
          });

        _serverInstance.GetRanchDirector().AddRanchHorse(characterUid, horseUid);

        return {"A new horse has been added to your inventory.", "Restart your game for the changes to apply."};
      }

      return {"Unknown sub-literal"};
    });
}

void ChatSystem::RegisterAdminCommands()
{
  // users command
  _commandManager.RegisterCommand(
    "users",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
      if (not invokerRecord)
        return {"Server error"};

      bool isAdmin = false;
      invokerRecord.Immutable([&isAdmin](const data::Character& character)
      {
        isAdmin = character.role() != data::Character::Role::User;
      });

      if (not isAdmin)
        return {};

      // todo: implement only local check
      bool onlyLocal = false;
      if (arguments.size() > 0)
      {
        onlyLocal = arguments[0] == "local";
      }

      std::vector<std::string> userList;

      const auto& userInstances = _serverInstance.GetLobbyDirector().GetUsers();
      userList.emplace_back(std::format("Users ({}):", userInstances.size()));

      for (const auto& userInstance : userInstances | std::views::values)
      {
        userList.emplace_back(std::format(
          UserLine,
          userInstance.userName));

        const auto userRecord = _serverInstance.GetDataDirector().GetUser(
          userInstance.userName);
        if (userRecord)
        {
          auto onlineCharacterUid{data::InvalidUid};
          userRecord.Immutable([&onlineCharacterUid](const data::User& user)
          {
            onlineCharacterUid = user.characterUid();
          });

          const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
            onlineCharacterUid);
          if (characterRecord)
          {
            characterRecord.Immutable([&userList](const data::Character& character)
            {
              userList.emplace_back(std::format(
                CharacterLine,
                character.uid(),
                character.name(),
                character.level()));
            });
          }
          else
          {
            userList.emplace_back(std::format(
              NoCharacterLine));
          }
        }
      }

      return userList;
    });

  // notice command
  _commandManager.RegisterCommand(
    "notice",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
      if (not invokerRecord)
        return {"Server error"};

      bool isAdmin = false;
      invokerRecord.Immutable([&isAdmin](const data::Character& character)
      {
        isAdmin = character.role() != data::Character::Role::User;
      });

      if (not isAdmin)
        return {};

      if (arguments.size() < 2)
      {
        return {"notice",
        "  [character UID]",
        "  - Specify 0 to send to all"
        "  [message]"};
      }

      std::string message;
      for (const auto& word : arguments.subspan(1))
      {
        message += word;
        message += " ";
      }

      const data::Uid specifiedCharacterUid = std::atoi(arguments[0].data());
      if (specifiedCharacterUid != data::InvalidUid)
      {
        const auto onlineCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
          specifiedCharacterUid);
        if (not onlineCharacterRecord)
        {
          return {"Character unavailable or offline"};
        }

        _serverInstance.GetLobbyDirector().NotifyCharacter(specifiedCharacterUid, message);
        return {"Notice sent to character"};
      }

      for (const auto& userInstance : _serverInstance.GetLobbyDirector().GetUsers() | std::views::values)
      {
        _serverInstance.GetLobbyDirector().NotifyCharacter(userInstance.characterUid, message);
      }
      return {"Notice sent to all characters"};
    });

  // promote command
  _commandManager.RegisterCommand(
    "promote",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
      if (not invokerRecord)
        return {"Server error"};

      bool isAdmin = false;
      invokerRecord.Immutable([&isAdmin](const data::Character& character)
      {
        isAdmin = character.role() != data::Character::Role::User;
      });

      if (not isAdmin)
        return {};

      if (arguments.empty())
      {
        return {"Specify user name"};
      }

      const auto userName = arguments[0];

      const auto userInstance = _serverInstance.GetLobbyDirector().GetUser(userName);
      if (not _serverInstance.GetLobbyDirector().IsUserOnline(userName))
      {
        return {std::format("User '{}' is not online", userName)};
      }

      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        userInstance.characterUid);
      if (not characterRecord)
      {
        return {std::format("User '{}' does not have a character", userName)};
      }

      std::string characterName;
      characterRecord.Mutable([&characterName](data::Character& character)
      {
        character.role() = data::Character::Role::GameMaster;
        characterName = character.name();
      });

      return {std::format("User '{}' ({}) promoted to GM", userName, characterName)};
    });

  // demote command
  _commandManager.RegisterCommand(
    "demote",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
      if (not invokerRecord)
        return {"Server error"};

      bool isAdmin = false;
      invokerRecord.Immutable([&isAdmin](const data::Character& character)
      {
        isAdmin = character.role() != data::Character::Role::User;
      });

      if (not isAdmin)
        return {};

      if (arguments.empty())
      {
        return {"Specify user name"};
      }

      const auto userName = arguments[0];

      const auto userInstance = _serverInstance.GetLobbyDirector().GetUser(userName);
      if (not _serverInstance.GetLobbyDirector().IsUserOnline(userName))
      {
        return {std::format("User '{}' is not online", userName)};
      }

      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        userInstance.characterUid);
      if (not characterRecord)
      {
        return {std::format("User '{}' does not have a character", userName)};
      }

      std::string characterName;
      characterRecord.Mutable([&characterName](data::Character& character)
      {
        character.role() = data::Character::Role::User;
        characterName = character.name();
      });

      return {std::format("User '{}' ({}) demoted to user", userName, characterName)};
    });

  // infraction command
  _commandManager.RegisterCommand(
    "infraction",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
      if (not invokerRecord)
        return {"Server error"};

      bool isAdmin = false;
      invokerRecord.Immutable([&isAdmin](const data::Character& character)
      {
        isAdmin = character.role() != data::Character::Role::User;
      });

      if (not isAdmin)
        return {};

      if (arguments.empty())
        return {"infraction",
          "  [add/remove/list]"};

      const std::string subLiteral = arguments[0];

      if (subLiteral == "add")
      {
        if (arguments.size() < 4)
        {
          return {
            "infraction add",
            "  [user name]",
            "  [none/mute/ban]",
            "  [duration (XXmXXhXXd or forever)]",
            "  [optional: description]"};
        }

        const std::string userName = arguments[1];
        const auto userRecord = _serverInstance.GetDataDirector().GetUser(userName);
        if (not userRecord)
        {
          return {"User not available"};
        }

        const std::string type = arguments[2];
        data::Infraction::Punishment punishmentType;
        if (type == "mute")
          punishmentType = data::Infraction::Punishment::Mute;
        else if (type == "ban")
          punishmentType = data::Infraction::Punishment::Ban;
        else
          punishmentType = data::Infraction::Punishment::None;

        const std::string durationInput = arguments[3];
        data::Clock::duration duration = std::chrono::seconds::zero();

        const std::regex minutePattern(R"((\d+)m)");
        const std::regex hourPattern(R"((\d+)h)");
        const std::regex dayPattern(R"((\d+)d)");
        std::smatch match;

        if (durationInput == "forever")
        {
          duration = data::Clock::duration::max();
        }
        else
        {
          if (std::regex_search(durationInput, match, minutePattern)) {
            duration += std::chrono::minutes(std::stoi(match[1].str()));
          }
          if (std::regex_search(durationInput, match, hourPattern)) {
            duration += std::chrono::hours(std::stoi(match[1].str()));
          }
          if (std::regex_search(durationInput, match, dayPattern)) {
            duration += std::chrono::days(std::stoi(match[1].str()));
          }
        }

        if (duration == duration.zero())
        {
          return {"Invalid duration, format example: 20m10h1d or forever"};
        }

        std::string description;
        if (arguments.size() > 4)
        {
          for (const std::string& word : arguments.subspan(4))
          {
            description += word;
            description += " ";
          }
        }

        auto infractionUid{data::InvalidUid};
        const auto infractionRecord = _serverInstance.GetDataDirector().CreateInfraction();
        infractionRecord.Mutable(
          [&infractionUid, punishmentType, duration, description](
            data::Infraction& infraction)
          {
            infraction.punishment = punishmentType;
            infraction.duration = duration;
            infraction.description = description;
            infraction.createdAt = data::Clock::now();

            infractionUid = infraction.uid();
          });

        auto userCharacterUid{data::InvalidUid};
        userRecord.Mutable([infractionUid, &userCharacterUid](data::User& user)
        {
          user.infractions().emplace_back(infractionUid);

          userCharacterUid = user.characterUid();
        });

        if (punishmentType == data::Infraction::Punishment::Ban)
        {
          _serverInstance.GetLobbyDirector().DisconnectCharacter(userCharacterUid);
          _serverInstance.GetRanchDirector().Disconnect(userCharacterUid);
          _serverInstance.GetRaceDirector().DisconnectCharacter(userCharacterUid);
        }
        else if (punishmentType == data::Infraction::Punishment::Mute)
        {
          _serverInstance.GetLobbyDirector().MuteCharacter(userCharacterUid, data::Clock::now() + duration);
        }

        return {std::format("Infraction added to '{}'", userName)};
      }
      else if (subLiteral == "remove")
      {
        if (arguments.size() < 3)
        {
          return {"infraction remove",
            "  [user name]",
            "  [infraction UID]"};
        }

        const std::string userName = arguments[1];
        const auto userRecord = _serverInstance.GetDataDirector().GetUser(userName);
        if (not userRecord)
        {
          return {"User not available"};
        }

        const data::Uid infractionUid = std::atoi(arguments[2].c_str());
        bool hasInfraction = false;

        userRecord.Mutable([infractionUid, &hasInfraction](data::User& user)
        {
          hasInfraction = std::ranges::contains(user.infractions(), infractionUid);

          if (hasInfraction)
          {
            const auto range = std::ranges::remove(user.infractions(), infractionUid);
            user.infractions().erase(range.begin(), range.end());
          }
        });

        if (not hasInfraction)
          return {"No such infraction exists"};

        return {std::format("Infraction removed from '{}'", userName)};
      }
      else if (subLiteral == "list")
      {
        if (arguments.size() < 2)
        {
          return {"infraction list",
            "  [user name]"};
        }

        const std::string userName = arguments[1];
        const auto userRecord = _serverInstance.GetDataDirector().GetUser(userName);
        if (not userRecord)
        {
          return {"User not available"};
        }

        std::vector<std::string> list;
        list.emplace_back(std::format("Infractions of '{}':", userName));

        userRecord.Immutable([this, &list](const data::User& user)
        {
          const auto infractionRecords = _serverInstance.GetDataDirector().GetInfractionCache().Get(
            user.infractions());
          if (not infractionRecords)
          {
            list.emplace_back("Infractions unavailable");
            return;
          }

          for (const auto& infractionRecord : *infractionRecords)
          {
            infractionRecord.Immutable([&list](const data::Infraction& infraction)
            {
              list.emplace_back(std::format(
                " - UID: #{} - {}", infraction.uid(), infraction.description()));

              std::string type;
              if (infraction.punishment() == data::Infraction::Punishment::None)
                type = "none";
              else if (infraction.punishment() == data::Infraction::Punishment::Mute)
                type = "<font color=\"#FF0000\">mute</font>";
              else if (infraction.punishment() == data::Infraction::Punishment::Ban)
                type = "<font color=\"#FF0000\">ban</font>";

              list.emplace_back(std::format(
                "   punishment: {}", type));

              {
                const auto expires = infraction.createdAt() + infraction.duration();

                const std::chrono::year_month_day date{
                  std::chrono::floor<std::chrono::days>(expires)};
                const std::chrono::hh_mm_ss time{
                  expires - std::chrono::floor<std::chrono::days>(expires)};

                list.emplace_back(std::format(
                  "   expires: {}/{}/{} - {:02}:{:02} (UTC)",
                  date.day(),
                  date.month(),
                  date.year(),
                  time.hours().count(),
                  time.minutes().count()));
              }

              {
                const std::chrono::year_month_day date{std::chrono::floor<std::chrono::days>(
                  infraction.createdAt())};
                const std::chrono::hh_mm_ss time{
                  infraction.createdAt() - std::chrono::floor<std::chrono::days>(infraction.createdAt())};

                list.emplace_back(std::format(
                  "   created: {}/{}/{} - {:02}:{:02} (UTC)",
                  date.day(),
                  date.month(),
                  date.year(),
                  time.hours().count(),
                  time.minutes().count()));
              }
            });
          }
        });

        return list;
      }

      return {"Unknown sub literal"};
    });

  // incognito command
  _commandManager.RegisterCommand(
    "incognito",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
      if (not invokerRecord)
        return {"Server error"};

      bool isAdmin = false;
      invokerRecord.Immutable([&isAdmin](const data::Character& character)
      {
        isAdmin = character.role() != data::Character::Role::User;
      });

      if (not isAdmin)
        return {};

      bool wasIncognito = false;
      invokerRecord.Mutable([&wasIncognito](data::Character& character)
      {
        wasIncognito = character.role() == data::Character::Role::Op;

        if (wasIncognito)
        {
          character.role() = data::Character::Role::GameMaster;
        }
        else
        {
          character.role() = data::Character::Role::Op;
        }
      });

      if (wasIncognito)
      {
        return {"Incognito mode turned off"};
      }

      return {"Incognito mode turned on"};
    });

  // info command
  _commandManager.RegisterCommand(
    "info",
    [this](
      const std::span<const std::string>& arguments,
      data::Uid characterUid) -> std::vector<std::string>
    {
      const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
      if (not invokerRecord)
        return {"Server error"};

      bool isAdmin = false;
      invokerRecord.Immutable([&isAdmin](const data::Character& character)
      {
        isAdmin = character.role() != data::Character::Role::User;
      });

      if (not isAdmin)
        return {};

      if (arguments.empty())
        return {"info",
          " [user/character] [name]"};

      const auto subject = arguments[0];

      const auto dumpCharacterInfo = [this](const data::Character& character)
      {
        const auto userInstance = _serverInstance.GetLobbyDirector().GetUserByCharacterUid(
          character.uid());

        std::vector<std::string> response;
        response.emplace_back(std::format("'{}' ({}) is user '{}'", character.name(), character.uid(), userInstance.userName));

        if (character.guildUid() != data::InvalidUid)
        {
          const auto guildRecord = _serverInstance.GetDataDirector().GetGuild(
            character.guildUid());
          if (guildRecord)
          {
            guildRecord.Immutable([&response, characterUid = character.uid()](
              const data::Guild& guild)
            {
              if (guild.owner() == characterUid)
              {
                response.emplace_back(std::format(
                  "Owner of a guild '{}' ({})",
                  guild.name(),
                  guild.uid()));
              }
              else if (std::ranges::contains(guild.officers(), characterUid))
              {
                response.emplace_back(std::format(
                  "Officer of a guild '{}' ({})",
                  guild.name(),
                  guild.uid()));
              }
              else
              {
                response.emplace_back(std::format(
                  "Member of a guild '{}' ({})",
                  guild.name(),
                  guild.uid()));
              }
            });
          }
          else
          {
            response.emplace_back(std::format("Member of unavailable guild {}", character.guildUid()));
          }
        }

        if (userInstance.roomUid > 0)
          response.emplace_back(std::format("Currently in a room {}", userInstance.roomUid));
        else
          response.emplace_back(std::format("Currently at a ranch"));

        response.emplace_back(std::format("Horses:"));

        const auto mountRecord = _serverInstance.GetDataDirector().GetHorse(character.mountUid());
        if (mountRecord)
        {
          mountRecord.Immutable([&response](const data::Horse& horse)
          {
            response.emplace_back(std::format(" > '{}' ({})", horse.name(), horse.uid()));
          });
        }
        else
        {
          response.emplace_back(" * mount not available");
        }

        const auto horseRecords = _serverInstance.GetDataDirector().GetHorseCache().Get(
          character.horses());

        if (horseRecords)
        {
          for (const auto& horseRecord : *horseRecords)
          {
            horseRecord.Immutable([&response](const data::Horse& horse)
            {
              response.emplace_back(std::format(" - {} ({})", horse.name(), horse.uid()));
            });
          }
        }
        else
        {
          response.emplace_back(" - horses not available");
        }

        return response;
      };

      if (subject == "character")
      {
        if (arguments.size() < 2)
        {
          return {"info character [name]"};
        }

        const auto name = arguments[1];
        for (const auto& [userName, userInstance] : _serverInstance.GetLobbyDirector().GetUsers())
        {
          const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
            userInstance.characterUid);
          if (not characterRecord)
            continue;

          std::string characterName;
          characterRecord.Immutable([&characterName](const data::Character& character)
          {
            characterName = character.name();
          });

          if (characterName != name)
            continue;

          std::vector<std::string> response;
          characterRecord.Immutable([&response, &dumpCharacterInfo](const data::Character& character)
          {
            response = dumpCharacterInfo(character);
          });

          return response;
        }

        return {std::format("Character '{}' is not online", name)};
      }
      else if (subject == "user")
      {
        if (arguments.size() < 2)
        {
          return {"info user [name]"};
        }

        const auto name = arguments[1];
        if (not _serverInstance.GetLobbyDirector().IsUserOnline(name))
        {
          return {std::format("User '{}' is not online", name)};
        }

        const auto& userInstance = _serverInstance.GetLobbyDirector().GetUser(name);
        const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
          userInstance.characterUid);

        if (not characterRecord)
        {
          return {std::format("User '{}' does not have a character", name)};
        }

        std::vector<std::string> response;
        characterRecord.Immutable([&response, &dumpCharacterInfo](
          const data::Character& character)
        {
          response = dumpCharacterInfo(character);
        });

        return response;
      }

      return {"Unknown sub-literal"};
    });
}

void ChatSystem::Broadcast(
  std::string message)
{
}

} // namespace server
