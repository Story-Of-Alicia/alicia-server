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

#include "libserver/network/command/proto/CommonStructureDefinitions.hpp"

#include <cassert>

namespace server::protocol
{

void Item::Write(const Item& item, SinkStream& stream)
{
  stream.Write(item.uid)
    .Write(item.tid)
    .Write(item.expiresAt)
    .Write(item.count);
}

void Item::Read(Item& item, SourceStream& stream)
{
  stream.Read(item.uid)
    .Read(item.tid)
    .Read(item.expiresAt)
    .Read(item.count);
}

void StoredItem::Write(const StoredItem& item, SinkStream& stream)
{
  stream.Write(item.uid)
    .Write(item.val1)
    .Write(item.status)
    .Write(item.val3)
    .Write(item.val4)
    .Write(item.val5)
    .Write(item.val6)
    .Write(item.sender)
    .Write(item.message)
    .Write(item.dateAndTime);
}

void StoredItem::Read(StoredItem& item, SourceStream& stream)
{
  stream.Read(item.uid)
    .Read(item.val1)
    .Read(item.status)
    .Read(item.val3)
    .Read(item.val4)
    .Read(item.val5)
    .Read(item.val6)
    .Read(item.sender)
    .Read(item.message)
    .Read(item.dateAndTime);
}

void KeyboardOptions::Option::Write(const Option& option, SinkStream& stream)
{
  stream.Write(option.secondaryKey)
    .Write(option.type)
    .Write(option.unused)
    .Write(option.primaryKey);
}

void KeyboardOptions::Option::Read(Option& option, SourceStream& stream)
{
  stream.Read(option.secondaryKey)
    .Read(option.type)
    .Read(option.unused)
    .Read(option.primaryKey);
}

void KeyboardOptions::Write(const KeyboardOptions& value, SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(value.bindings.size()));

  for (const auto& binding : value.bindings)
  {
    stream.Write(binding);
  }
}

void KeyboardOptions::Read(KeyboardOptions& value, SourceStream& stream)
{
  uint8_t size;
  stream.Read(size);
  value.bindings.resize(size);

  for (auto& binding : value.bindings)
  {
    stream.Read(binding);
  }
}

void MacroOptions::Write(const MacroOptions& value, SinkStream& stream)
{
  for (const auto& macro : value.macros)
  {
    stream.Write(macro);
  }
}

void MacroOptions::Read(MacroOptions& value, SourceStream& stream)
{
  for (auto& macro : value.macros)
  {
    stream.Read(macro);
  }
}

void GamepadOptions::Option::Write(const Option& option, SinkStream& stream)
{
  stream.Write(option.secondaryButton)
    .Write(option.type)
    .Write(option.unused)
    .Write(option.primaryButton);
}

void GamepadOptions::Option::Read(Option& option, SourceStream& stream)
{
  stream.Read(option.secondaryButton)
    .Read(option.type)
    .Read(option.unused)
    .Read(option.primaryButton);
}

void GamepadOptions::Write(const GamepadOptions& value, SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(value.bindings.size()));

  for (const auto& binding : value.bindings)
  {
    stream.Write(binding);
  }
}

void GamepadOptions::Read(GamepadOptions& value, SourceStream& stream)
{
  uint8_t size;
  stream.Read(size);
  value.bindings.resize(size);

  for (auto& binding : value.bindings)
  {
    stream.Read(binding);
  }
}

void Settings::Write(const Settings& value, SinkStream& stream)
{
  uint32_t typeValue{value.typeBitset.to_ulong()};
  stream.Write(typeValue);

  // Write the keyboard options if specified in the option type mask.
  if (value.typeBitset.test(Type::Keyboard))
  {
    const auto& keyboard = value.keyboardOptions;
    stream.Write(static_cast<uint8_t>(keyboard.bindings.size()));

    for (const auto& binding : keyboard.bindings)
    {
      stream.Write(binding.type)
        .Write(binding.unused)
        .Write(binding.primaryKey)
        .Write(binding.secondaryKey);
    }
  }

  // Write the macro options if specified in the option type mask.
  if (value.typeBitset.test(Type::Macros))
  {
    const auto& macros = value.macroOptions;

    for (const auto& macro : macros.macros)
    {
      stream.Write(macro);
    }
  }

  // Write the value option if specified in the option type mask.
  if (value.typeBitset.test(Type::Value))
  {
    stream.Write(value.valueOption);
  }

  // Write the gamepad options if specified in the option type mask.
  if (value.typeBitset.test(Type::Gamepad))
  {
    const auto& gamepad = value.gamepadOptions;
    stream.Write(static_cast<uint8_t>(gamepad.bindings.size()));

    for (const auto& binding : gamepad.bindings)
    {
      stream.Write(binding.type)
        .Write(binding.unused)
        .Write(binding.primaryButton)
        .Write(binding.secondaryButton);
    }
  }

  stream.Write(value.age)
    .Write(value.hideAge);
}

void Settings::Read(Settings& value, SourceStream& stream)
{
  uint32_t typeBitsetValue;
  stream.Read(typeBitsetValue);

  value.typeBitset = typeBitsetValue;

  // Write the keyboard options if specified in the option type mask.
  if (value.typeBitset.test(Type::Keyboard))
  {
    auto& keyboard = value.keyboardOptions;
    uint8_t bindingCount = 0;
    stream.Read(bindingCount);
    keyboard.bindings.resize(bindingCount);

    for (auto& binding : keyboard.bindings)
    {
      stream.Read(binding.type)
        .Read(binding.unused)
        .Read(binding.primaryKey)
        .Read(binding.secondaryKey);
    }
  }

  // Write the gamepad options if specified in the option type mask.
  if (value.typeBitset.test(Type::Gamepad))
  {
    auto& gamepad = value.gamepadOptions;
    uint8_t bindingCount = 0;
    stream.Read(bindingCount);
    gamepad.bindings.resize(bindingCount);

    for (auto& binding : gamepad.bindings)
    {
      stream.Read(binding.type)
        .Read(binding.unused)
        .Read(binding.primaryButton)
        .Read(binding.secondaryButton);
    }
  }

  // Write the macro options if specified in the option type mask.
  if (value.typeBitset.test(Type::Macros))
  {
    auto& macros = value.macroOptions;

    for (auto& macro : macros.macros)
    {
      stream.Read(macro);
    }
  }

  // Write the value option if specified in the option type mask.
  if (value.typeBitset.test(Type::Value))
  {
    stream.Read(value.valueOption);
  }

  stream.Read(value.age)
    .Read(value.hideAge);
}

void Character::Parts::Write(const Parts& value, SinkStream& stream)
{
  stream.Write(value.charId)
    .Write(value.mouthSerialId)
    .Write(value.faceSerialId)
    .Write(value.val0);
}

void Character::Parts::Read(Parts& value, SourceStream& stream)
{
  stream.Read(value.charId)
    .Read(value.mouthSerialId)
    .Read(value.faceSerialId)
    .Read(value.val0);
}

void Character::Appearance::Write(const Appearance& value, SinkStream& stream)
{
  stream.Write(value.voiceId)
    .Write(value.headSize)
    .Write(value.height)
    .Write(value.thighVolume)
    .Write(value.legVolume)
    .Write(value.emblemId);
}

void Character::Appearance::Read(Appearance& value, SourceStream& stream)
{
  stream.Read(value.voiceId)
    .Read(value.headSize)
    .Read(value.height)
    .Read(value.thighVolume)
    .Read(value.legVolume)
    .Read(value.emblemId);
}

void Character::Write(const Character& value, SinkStream& stream)
{
  stream.Write(value.parts)
    .Write(value.appearance);
}

void Character::Read(Character& value, SourceStream& stream)
{
  stream.Read(value.parts)
    .Read(value.appearance);
}

void Horse::Parts::Write(const Parts& value, SinkStream& stream)
{
  stream.Write(value.skinId)
    .Write(value.maneId)
    .Write(value.tailId)
    .Write(value.faceId);
}

void Horse::Parts::Read(Parts& value, SourceStream& stream)
{
  stream.Read(value.skinId)
    .Read(value.maneId)
    .Read(value.tailId)
    .Read(value.faceId);
}

void Horse::Appearance::Write(const Appearance& value, SinkStream& stream)
{
  stream.Write(value.scale)
    .Write(value.legLength)
    .Write(value.legVolume)
    .Write(value.bodyLength)
    .Write(value.bodyVolume);
}

void Horse::Appearance::Read(Appearance& value, SourceStream& stream)
{
  stream.Read(value.scale)
    .Read(value.legLength)
    .Read(value.legVolume)
    .Read(value.bodyLength)
    .Read(value.bodyVolume);
}

void Horse::Stats::Write(const Stats& value, SinkStream& stream)
{
  stream.Write(value.agility)
    .Write(value.ambition)
    .Write(value.rush)
    .Write(value.endurance)
    .Write(value.courage);
}

void Horse::Stats::Read(Stats& value, SourceStream& stream)
{
  stream.Read(value.agility)
    .Read(value.ambition)
    .Read(value.rush)
    .Read(value.endurance)
    .Read(value.courage);
}

void Horse::Mastery::Write(const Mastery& value, SinkStream& stream)
{
  stream.Write(value.spurMagicCount)
    .Write(value.jumpCount)
    .Write(value.slidingTime)
    .Write(value.glidingDistance);
}

void Horse::Mastery::Read(Mastery& value, SourceStream& stream)
{
  stream.Read(value.spurMagicCount)
    .Read(value.jumpCount)
    .Read(value.slidingTime)
    .Read(value.glidingDistance);
}

void Horse::Write(const Horse& value, SinkStream& stream)
{
  stream.Write(value.uid)
    .Write(value.tid)
    .Write(value.name);

  stream.Write(value.parts)
    .Write(value.appearance)
    .Write(value.stats);

  stream.Write(value.rating)
    .Write(value.clazz)
    .Write(value.val0)
    .Write(value.grade)
    .Write(value.growthPoints);

  stream.Write(value.mountCondition.stamina)
    .Write(value.mountCondition.charmPoint)
    .Write(value.mountCondition.friendlyPoint)
    .Write(value.mountCondition.injuryPoint)
    .Write(value.mountCondition.plenitude)
    .Write(value.mountCondition.bodyDirtiness)
    .Write(value.mountCondition.maneDirtiness)
    .Write(value.mountCondition.tailDirtiness)
    .Write(value.mountCondition.attachment)
    .Write(value.mountCondition.boredom)
    .Write(value.mountCondition.bodyPolish)
    .Write(value.mountCondition.manePolish)
    .Write(value.mountCondition.tailPolish)
    .Write(value.mountCondition.stopAmendsPoint);

  stream.Write(value.vals1.type)
    .Write(value.vals1.val1)
    .Write(value.vals1.dateOfBirth)
    .Write(value.vals1.tendency)
    .Write(value.vals1.spirit)
    .Write(value.vals1.classProgression)
    .Write(value.vals1.val5)
    .Write(value.vals1.potentialLevel)
    .Write(value.vals1.potentialType)
    .Write(value.vals1.potentialValue)
    .Write(value.vals1.val9)
    .Write(value.vals1.luck)
    .Write(value.vals1.injury)
    .Write(value.vals1.val12)
    .Write(value.vals1.fatigue)
    .Write(value.vals1.val14)
    .Write(value.vals1.emblem);

  stream.Write(value.mastery);

  stream.Write(value.val16)
    .Write(value.visualCleanlinessBitset);
}

void Horse::Read(Horse& value, SourceStream& stream)
{
  stream.Read(value.uid)
    .Read(value.tid)
    .Read(value.name);

  stream.Read(value.parts)
    .Read(value.appearance)
    .Read(value.stats);

  stream.Read(value.rating)
    .Read(value.clazz)
    .Read(value.val0)
    .Read(value.grade)
    .Read(value.growthPoints);

  stream.Read(value.mountCondition.stamina)
    .Read(value.mountCondition.charmPoint)
    .Read(value.mountCondition.friendlyPoint)
    .Read(value.mountCondition.injuryPoint)
    .Read(value.mountCondition.plenitude)
    .Read(value.mountCondition.bodyDirtiness)
    .Read(value.mountCondition.maneDirtiness)
    .Read(value.mountCondition.tailDirtiness)
    .Read(value.mountCondition.attachment)
    .Read(value.mountCondition.boredom)
    .Read(value.mountCondition.bodyPolish)
    .Read(value.mountCondition.manePolish)
    .Read(value.mountCondition.tailPolish)
    .Read(value.mountCondition.stopAmendsPoint);

  stream.Read(value.vals1.type)
    .Read(value.vals1.val1)
    .Read(value.vals1.dateOfBirth)
    .Read(value.vals1.tendency)
    .Read(value.vals1.spirit)
    .Read(value.vals1.classProgression)
    .Read(value.vals1.val5)
    .Read(value.vals1.potentialLevel)
    .Read(value.vals1.potentialType)
    .Read(value.vals1.potentialValue)
    .Read(value.vals1.val9)
    .Read(value.vals1.luck)
    .Read(value.vals1.injury)
    .Read(value.vals1.val12)
    .Read(value.vals1.fatigue)
    .Read(value.vals1.val14)
    .Read(value.vals1.emblem);

  stream.Read(value.mastery);

  stream.Read(value.val16)
    .Read(value.visualCleanlinessBitset);
}

void Guild::Write(const Guild& value, SinkStream& stream)
{
  stream.Write(value.uid)
    .Write(value.val1)
    .Write(value.val2)
    .Write(value.name)
    .Write(value.guildRole)
    .Write(value.val5)
    .Write(value.val6);
}

void Guild::Read(Guild& value, SourceStream& stream)
{
  stream.Read(value.uid)
    .Read(value.val1)
    .Read(value.val2)
    .Read(value.name)
    .Read(value.guildRole)
    .Read(value.val5)
    .Read(value.val6);
}

void Rent::Write(const Rent& value, SinkStream& stream)
{
  stream.Write(value.mountUid)
    .Write(value.val1)
    .Write(value.val2);
}

void Rent::Read(Rent& value, SourceStream& stream)
{
  stream.Read(value.mountUid)
    .Read(value.val1)
    .Read(value.val2);
}

void Pet::Write(const Pet& value, SinkStream& stream)
{
  stream.Write(value.petId)
    .Write(value.member2)
    .Write(value.name)
    .Write(value.birthDate);
}

void Pet::Read(Pet& value, SourceStream& stream)
{
  stream.Read(value.petId)
    .Read(value.member2)
    .Read(value.name)
    .Read(value.birthDate);
}

void Egg::Write(const Egg& value, SinkStream& stream)
{
  stream.Write(value.uid)
    .Write(value.itemTid)
    .Write(value.member3)
    .Write(value.member4)
    .Write(value.member5)
    .Write(value.timeRemaining)
    .Write(value.boost)
    .Write(value.totalHatchingTime)
    .Write(value.member9);
}

void Egg::Read(Egg& value, SourceStream& stream)
{
  stream.Read(value.uid)
    .Read(value.itemTid)
    .Read(value.member3)
    .Read(value.member4)
    .Read(value.member5)
    .Read(value.timeRemaining)
    .Read(value.boost)
    .Read(value.totalHatchingTime)
    .Read(value.member9);
}

void PetInfo::Write(const PetInfo& value, SinkStream& stream)
{
  stream.Write(value.characterUid)
    .Write(value.itemUid)
    .Write(value.pet)
    .Write(value.member4);
}

void PetInfo::Read(PetInfo& value, SourceStream& stream)
{
  stream.Read(value.characterUid)
    .Read(value.itemUid)
    .Read(value.pet)
    .Read(value.member4);
}

void PetBirthInfo::Write(const PetBirthInfo& value, SinkStream& stream)
{
  stream.Write(value.eggItem)
    .Write(value.member2)
    .Write(value.member3)
    .Write(value.petInfo);
}

void PetBirthInfo::Read(PetBirthInfo& value, SourceStream& stream)
{
  stream.Read(value.eggItem)
    .Read(value.member2)
    .Read(value.member3)
    .Read(value.petInfo);
}

void RanchHorse::Write(const RanchHorse& value, SinkStream& stream)
{
  stream.Write(value.horseOid)
    .Write(value.horse);
}

void RanchHorse::Read(RanchHorse& value, SourceStream& stream)
{
  stream.Read(value.horseOid)
    .Read(value.horse);
}

void RanchCharacter::Write(const RanchCharacter& ranchCharacter, SinkStream& stream)
{
  stream.Write(ranchCharacter.uid)
    .Write(ranchCharacter.name)
    .Write(ranchCharacter.role)
    .Write(ranchCharacter.age)
    .Write(ranchCharacter.gender)
    .Write(ranchCharacter.introduction);

  stream.Write(ranchCharacter.character)
    .Write(ranchCharacter.mount);

  stream.Write(static_cast<uint8_t>(ranchCharacter.characterEquipment.size()));
  for (const Item& item : ranchCharacter.characterEquipment)
  {
    stream.Write(item);
  }

  // Guild
  const auto& struct5 = ranchCharacter.guild;
  stream.Write(struct5.uid)
    .Write(struct5.val1)
    .Write(struct5.val2)
    .Write(struct5.name)
    .Write(struct5.guildRole)
    .Write(struct5.val5)
    .Write(struct5.val6);

  stream.Write(ranchCharacter.oid)
    .Write(ranchCharacter.isBusy)
    .Write(ranchCharacter.unk3);

  // Rent
  const auto& struct6 = ranchCharacter.rent;
  stream.Write(struct6.mountUid)
    .Write(struct6.val1)
    .Write(struct6.val2);

  // Pet
  const auto& struct7 = ranchCharacter.pet;
  stream.Write(ranchCharacter.pet)
    .Write(ranchCharacter.unk4)
    .Write(ranchCharacter.unk5);
}

void RanchCharacter::Read(RanchCharacter& value, SourceStream& stream)
{
  stream.Read(value.uid)
    .Read(value.name)
    .Read(reinterpret_cast<uint8_t&>(value.role))
    .Read(value.age)
    .Read(value.gender)
    .Read(value.introduction);

  stream.Read(value.character).Read(value.mount);

  uint8_t size;
  stream.Read(size);
  value.characterEquipment.resize(size);
  for (auto& item : value.characterEquipment)
  {
    stream.Read(item);
  }

  stream.Read(value.guild);

  stream.Read(value.oid)
    .Read(value.isBusy)
    .Read(value.unk3);

  stream.Read(value.rent)
    .Read(value.pet);

  stream.Read(value.unk4)
    .Read(value.unk5);
}

void Quest::Write(const Quest& value, SinkStream& stream)
{
  stream.Write(value.tid)
    .Write(value.member0)
    .Write(value.member1)
    .Write(value.member2)
    .Write(value.member3)
    .Write(value.member4);
}

void Quest::Read(Quest& value, SourceStream& stream)
{
  stream.Read(value.tid)
    .Read(value.member0)
    .Read(value.member1)
    .Read(value.member2)
    .Read(value.member3)
    .Read(value.member4);
}

void Housing::Write(const Housing& value, SinkStream& stream)
{
  stream.Write(value.uid)
    .Write(value.tid)
    .Write(value.durability);
}

void Housing::Read(Housing& value, SourceStream& stream)
{
  stream.Read(value.uid)
    .Read(value.tid)
    .Read(value.durability);
}

void League::Write(const League& value, SinkStream& stream)
{
  stream.Write(value.type)
    .Write(value.rankingPercentile);
}

void League::Read(League& value, SourceStream& stream)
{
  stream.Read(value.type)
    .Read(value.rankingPercentile);
}

void SkillSet::Write(const SkillSet& value, SinkStream& stream)
{
  // Set 1 or 2 are supported
  // TODO: enable this if we are certain only max of 2 sets are possible
  // assert(value.setId < 3);
  // Only magic or speed skills are saved (see tag10 @ 0x0050f760)
  // Note: Gamemode 4 (spectator?) was discovered doing some auxilary function
  assert(
    value.gamemode == GameMode::Magic ||
    value.gamemode == GameMode::Speed ||
    value.gamemode == GameMode::Unk4);
  // Updating a skill set requires 2 skill values (can be 0) to be sent
  assert(value.skills.size() == 2);

  stream.Write(value.setId);
  // Gamemode needs recasting to uint32_t for the command
  stream.Write(static_cast<uint32_t>(value.gamemode)); 
  
  stream.Write(static_cast<uint8_t>(value.skills.size()));
  for (const auto& skill : value.skills)
  {
    stream.Write(skill);
  }
}

void SkillSet::Read(SkillSet& value, SourceStream& stream)
{
  // Command provides gamemode as uint32_t, recast it to its enum
  uint32_t commandGameMode;
  stream.Read(value.setId)
    .Read(commandGameMode);
  value.gamemode = static_cast<GameMode>(commandGameMode);

  uint8_t size;
  stream.Read(size);
  value.skills.resize(size);
  for (auto& element : value.skills)
  {
    stream.Read(element);
  }
}

} // namespace server::protocol
