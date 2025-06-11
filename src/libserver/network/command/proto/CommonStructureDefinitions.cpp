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

namespace alicia
{

void Item::Write(const Item& item, SinkStream& stream)
{
  stream.Write(item.uid)
    .Write(item.tid)
    .Write(item.val)
    .Write(item.count);
}

void Item::Read(Item& item, SourceStream& stream)
{
  stream.Read(item.uid)
    .Read(item.tid)
    .Read(item.val)
    .Read(item.count);
}

void KeyboardOptions::Option::Write(const Option& option, SinkStream& stream)
{
  stream.Write(option.index)
    .Write(option.type)
    .Write(option.key);
}

void KeyboardOptions::Option::Read(Option& option, SourceStream& stream)
{
  stream.Read(option.index)
    .Read(option.type)
    .Read(option.key);
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
  stream.Write(value.val0)
    .Write(value.headSize)
    .Write(value.height)
    .Write(value.thighVolume)
    .Write(value.legVolume)
    .Write(value.val1);
}

void Character::Appearance::Read(Appearance& value, SourceStream& stream)
{
  stream.Read(value.val0)
    .Read(value.headSize)
    .Read(value.height)
    .Read(value.thighVolume)
    .Read(value.legVolume)
    .Read(value.val1);
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
    .Write(value.control)
    .Write(value.speed)
    .Write(value.strength)
    .Write(value.spirit);
}

void Horse::Stats::Read(Stats& value, SourceStream& stream)
{
  stream.Read(value.agility)
    .Read(value.control)
    .Read(value.speed)
    .Read(value.strength)
    .Read(value.spirit);
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

  stream.Write(value.vals0.stamina)
    .Write(value.vals0.attractiveness)
    .Write(value.vals0.hunger)
    .Write(value.vals0.val0)
    .Write(value.vals0.val1)
    .Write(value.vals0.val2)
    .Write(value.vals0.val3)
    .Write(value.vals0.val4)
    .Write(value.vals0.val5)
    .Write(value.vals0.val6)
    .Write(value.vals0.val7)
    .Write(value.vals0.val8)
    .Write(value.vals0.val9)
    .Write(value.vals0.val10);

  stream.Write(value.vals1.val0)
    .Write(value.vals1.val1)
    .Write(value.vals1.dateOfBirth)
    .Write(value.vals1.val3)
    .Write(value.vals1.val4)
    .Write(value.vals1.classProgression)
    .Write(value.vals1.val5)
    .Write(value.vals1.potentialLevel)
    .Write(value.vals1.hasPotential)
    .Write(value.vals1.potentialValue)
    .Write(value.vals1.val9)
    .Write(value.vals1.luck)
    .Write(value.vals1.hasLuck)
    .Write(value.vals1.val12)
    .Write(value.vals1.fatigue)
    .Write(value.vals1.val14)
    .Write(value.vals1.emblem);

  stream.Write(value.mastery);

  stream.Write(value.val16).Write(value.val17);
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

  stream.Read(value.vals0.stamina)
    .Read(value.vals0.attractiveness)
    .Read(value.vals0.hunger)
    .Read(value.vals0.val0)
    .Read(value.vals0.val1)
    .Read(value.vals0.val2)
    .Read(value.vals0.val3)
    .Read(value.vals0.val4)
    .Read(value.vals0.val5)
    .Read(value.vals0.val6)
    .Read(value.vals0.val7)
    .Read(value.vals0.val8)
    .Read(value.vals0.val9)
    .Read(value.vals0.val10);

  stream.Read(value.vals1.val0)
    .Read(value.vals1.val1)
    .Read(value.vals1.dateOfBirth)
    .Read(value.vals1.val3)
    .Read(value.vals1.val4)
    .Read(value.vals1.classProgression)
    .Read(value.vals1.val5)
    .Read(value.vals1.potentialLevel)
    .Read(value.vals1.hasPotential)
    .Read(value.vals1.potentialValue)
    .Read(value.vals1.val9)
    .Read(value.vals1.luck)
    .Read(value.vals1.hasLuck)
    .Read(value.vals1.val12)
    .Read(value.vals1.fatigue)
    .Read(value.vals1.val14)
    .Read(value.vals1.emblem);

  stream.Read(value.mastery);

  stream.Read(value.val16)
    .Read(value.val17);
}

void Struct5::Write(const Struct5& value, SinkStream& stream)
{
  stream.Write(value.val0)
    .Write(value.val1)
    .Write(value.val2)
    .Write(value.val3)
    .Write(value.val4)
    .Write(value.val5)
    .Write(value.val6);
}

void Struct5::Read(Struct5& value, SourceStream& stream)
{
  stream.Read(value.val0)
    .Read(value.val1)
    .Read(value.val2)
    .Read(value.val3)
    .Read(value.val4)
    .Read(value.val5)
    .Read(value.val6);
}

void Struct6::Write(const Struct6& value, SinkStream& stream)
{
  stream.Write(value.mountUid)
    .Write(value.val1)
    .Write(value.val2);
}

void Struct6::Read(Struct6& value, SourceStream& stream)
{
  stream.Read(value.mountUid)
    .Read(value.val1)
    .Read(value.val2);
}

void Struct7::Write(const Struct7& value, SinkStream& stream)
{
  stream.Write(value.val0)
    .Write(value.val1)
    .Write(value.val2)
    .Write(value.val3);
}

void Struct7::Read(Struct7& value, SourceStream& stream)
{
  stream.Read(value.val0)
    .Read(value.val1)
    .Read(value.val2)
    .Read(value.val3);
}

void RanchHorse::Write(const RanchHorse& value, SinkStream& stream)
{
  stream.Write(value.ranchIndex)
    .Write(value.horse);
}

void RanchHorse::Read(RanchHorse& value, SourceStream& stream)
{
  stream.Read(value.ranchIndex)
    .Read(value.horse);
}

void RanchCharacter::Write(const RanchCharacter& ranchCharacter, SinkStream& stream)
{
  stream.Write(ranchCharacter.uid)
    .Write(ranchCharacter.name)
    .Write(ranchCharacter.gender)
    .Write(ranchCharacter.unk0)
    .Write(ranchCharacter.unk1)
    .Write(ranchCharacter.description);

  stream.Write(ranchCharacter.character)
    .Write(ranchCharacter.mount);

  stream.Write(static_cast<uint8_t>(ranchCharacter.characterEquipment.size()));
  for (const Item& item : ranchCharacter.characterEquipment)
  {
    stream.Write(item);
  }

  // Struct5
  const auto& struct5 = ranchCharacter.playerRelatedThing;
  stream.Write(struct5.val0)
    .Write(struct5.val1)
    .Write(struct5.val2)
    .Write(struct5.val3)
    .Write(struct5.val4)
    .Write(struct5.val5)
    .Write(struct5.val6);

  stream.Write(ranchCharacter.ranchIndex)
    .Write(ranchCharacter.unk2)
    .Write(ranchCharacter.unk3);

  // Struct6
  const auto& struct6 = ranchCharacter.anotherPlayerRelatedThing;
  stream.Write(struct6.mountUid)
    .Write(struct6.val1)
    .Write(struct6.val2);

  // Struct7
  const auto& struct7 = ranchCharacter.yetAnotherPlayerRelatedThing;
  stream.Write(struct7.val0)
    .Write(struct7.val1)
    .Write(struct7.val2)
    .Write(struct7.val3);

  stream.Write(ranchCharacter.unk4)
    .Write(ranchCharacter.unk5);
}

void RanchCharacter::Read(RanchCharacter& value, SourceStream& stream)
{
  stream.Read(value.uid)
    .Read(value.name)
    .Read(reinterpret_cast<uint8_t&>(value.gender))
    .Read(value.unk0)
    .Read(value.unk1)
    .Read(value.description);

  stream.Read(value.character).Read(value.mount);

  uint8_t size;
  stream.Read(size);
  value.characterEquipment.resize(size);
  for (auto& item : value.characterEquipment)
  {
    stream.Read(item);
  }

  stream.Read(value.playerRelatedThing);

  stream.Read(value.ranchIndex)
    .Read(value.unk2)
    .Read(value.unk3);

  stream.Read(value.anotherPlayerRelatedThing)
    .Read(value.yetAnotherPlayerRelatedThing);

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

void RanchUnk11::Write(const RanchUnk11& value, SinkStream& stream)
{
  stream.Write(value.unk0)
    .Write(value.unk1);
}

void RanchUnk11::Read(RanchUnk11& value, SourceStream& stream)
{
  stream.Read(value.unk0)
    .Read(value.unk1);
}

} // namespace alicia
