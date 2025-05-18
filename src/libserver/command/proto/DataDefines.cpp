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

#include "libserver/command/proto/DataDefines.hpp"

namespace alicia
{

void Item::Write(const Item& item, SinkStream& buffer)
{
    buffer.Write(item.uid)
        .Write(item.tid)
        .Write(item.val)
        .Write(item.count);
}

void Item::Read(Item& item, SourceStream& buffer)
{
    buffer.Read(item.uid)
        .Read(item.tid)
        .Read(item.val)
        .Read(item.count);
}

DEFINE_WRITER_READER(Item, Item::Write, Item::Read)


void KeyboardOptions::Option::Write(const Option& option, SinkStream& buffer)
{
    buffer.Write(option.index)
        .Write(option.type)
        .Write(option.key);
}

void KeyboardOptions::Option::Read(Option& option, SourceStream& buffer)
{
    buffer.Read(option.index)
        .Read(option.type)
        .Read(option.key);
}

DEFINE_WRITER_READER(KeyboardOptions::Option, KeyboardOptions::Option::Write, KeyboardOptions::Option::Read)


void KeyboardOptions::Write(const KeyboardOptions& value, SinkStream& buffer)
{
    buffer.Write(static_cast<uint8_t>(value.bindings.size()));
    for (const auto& binding : value.bindings)
    {
        buffer.Write(binding);
    }
}

void KeyboardOptions::Read(KeyboardOptions& value, SourceStream& buffer)
{
    uint8_t size;
    buffer.Read(size);
    value.bindings.resize(size);
    for (auto& binding : value.bindings)
    {
        buffer.Read(binding);
    }
}

DEFINE_WRITER_READER(KeyboardOptions, KeyboardOptions::Write, KeyboardOptions::Read)


void MacroOptions::Write(const MacroOptions& value, SinkStream& buffer)
{
    for (const auto& macro : value.macros)
    {
        buffer.Write(macro);
    }
}

void MacroOptions::Read(MacroOptions& value, SourceStream& buffer)
{
    for (auto& macro : value.macros)
    {
        buffer.Read(macro);
    }
}

DEFINE_WRITER_READER(MacroOptions, MacroOptions::Write, MacroOptions::Read)



void Character::Write(const Character& value, SinkStream& buffer)
{
  buffer.Write(value.member0)
    .Write(value.member1)
    .Write(value.member2)
    .Write(value.member3)
    .Write(value.member4);
}

void Character::Read(Character& value, SourceStream& buffer)
{
  buffer.Read(value.member0)
    .Read(value.member1)
    .Read(value.member2)
    .Read(value.member3)
    .Read(value.member4);
}

DEFINE_WRITER_READER(Character, Character::Write, Character::Read)


void Horse::Parts::Write(const Parts& value, SinkStream& buffer)
{
    buffer.Write(value.skinId)
        .Write(value.maneId)
        .Write(value.tailId)
        .Write(value.faceId);
}

void Horse::Parts::Read(Parts& value, SourceStream& buffer)
{
    buffer.Read(value.skinId)
        .Read(value.maneId)
        .Read(value.tailId)
        .Read(value.faceId);
}

DEFINE_WRITER_READER(Horse, Horse::Write, Horse::Read)


void Horse::Appearance::Write(const Appearance& value, SinkStream& buffer)
{
    buffer.Write(value.scale)
        .Write(value.legLength)
        .Write(value.legVolume)
        .Write(value.bodyLength)
        .Write(value.bodyVolume);
}

void Horse::Appearance::Read(Appearance& value, SourceStream& buffer)
{
    buffer.Read(value.scale)
        .Read(value.legLength)
        .Read(value.legVolume)
        .Read(value.bodyLength)
        .Read(value.bodyVolume);
}

DEFINE_WRITER_READER(Horse::Parts, Horse::Parts::Write, Horse::Parts::Read)


void Horse::Stats::Write(const Stats& value, SinkStream& buffer)
{
    buffer.Write(value.agility)
        .Write(value.spirit)
        .Write(value.speed)
        .Write(value.strength)
        .Write(value.ambition);
}

void Horse::Stats::Read(Stats& value, SourceStream& buffer)
{
    buffer.Read(value.agility)
        .Read(value.spirit)
        .Read(value.speed)
        .Read(value.strength)
        .Read(value.ambition);
}

DEFINE_WRITER_READER(Horse::Appearance, Horse::Appearance::Write, Horse::Appearance::Read)


void Horse::Mastery::Write(const Mastery& value, SinkStream& buffer)
{
    buffer.Write(value.magic)
        .Write(value.jumping)
        .Write(value.sliding)
        .Write(value.gliding);
}

void Horse::Mastery::Read(Mastery& value, SourceStream& buffer)
{
    buffer.Read(value.magic)
        .Read(value.jumping)
        .Read(value.sliding)
        .Read(value.gliding);
}

DEFINE_WRITER_READER(Horse::Stats, Horse::Stats::Write, Horse::Stats::Read)


void Horse::Write(const Horse& value, SinkStream& buffer)
{
    buffer.Write(value.uid)
        .Write(value.tid)
        .Write(value.name);

    buffer.Write(value.parts)
        .Write(value.appearance)
        .Write(value.stats);

    buffer.Write(value.rating)
        .Write(value.clazz)
        .Write(value.val0)
        .Write(value.grade)
        .Write(value.growthPoints);

    buffer.Write(value.vals0.stamina)
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

    buffer.Write(value.vals1.val0)
        .Write(value.vals1.val1)
        .Write(value.vals1.val2)
        .Write(value.vals1.val3)
        .Write(value.vals1.val4)
        .Write(value.vals1.classProgression)
        .Write(value.vals1.val5)
        .Write(value.vals1.val6)
        .Write(value.vals1.val7)
        .Write(value.vals1.val8)
        .Write(value.vals1.val9)
        .Write(value.vals1.val10)
        .Write(value.vals1.val11)
        .Write(value.vals1.val12)
        .Write(value.vals1.val13)
        .Write(value.vals1.val14)
        .Write(value.vals1.val15);

    buffer.Write(value.mastery);

    buffer.Write(value.val16)
        .Write(value.val17);
}

void Horse::Read(Horse& value, SourceStream& buffer)
{
    buffer.Read(value.uid)
        .Read(value.tid)
        .Read(value.name);

    buffer.Read(value.parts)
        .Read(value.appearance)
        .Read(value.stats);

    buffer.Read(value.rating)
        .Read(value.clazz)
        .Read(value.val0)
        .Read(value.grade)
        .Read(value.growthPoints);

    buffer.Read(value.vals0.stamina)
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

    buffer.Read(value.vals1.val0)
        .Read(value.vals1.val1)
        .Read(value.vals1.val2)
        .Read(value.vals1.val3)
        .Read(value.vals1.val4)
        .Read(value.vals1.classProgression)
        .Read(value.vals1.val5)
        .Read(value.vals1.val6)
        .Read(value.vals1.val7)
        .Read(value.vals1.val8)
        .Read(value.vals1.val9)
        .Read(value.vals1.val10)
        .Read(value.vals1.val11)
        .Read(value.vals1.val12)
        .Read(value.vals1.val13)
        .Read(value.vals1.val14)
        .Read(value.vals1.val15);

    buffer.Read(value.mastery);

    buffer.Read(value.val16)
        .Read(value.val17);
}

DEFINE_WRITER_READER(Horse::Mastery, Horse::Mastery::Write, Horse::Mastery::Read)


void Struct5::Write(const Struct5& value, SinkStream& buffer)
{
    buffer.Write(value.val0)
        .Write(value.val1)
        .Write(value.val2)
        .Write(value.val3)
        .Write(value.val4)
        .Write(value.val5)
        .Write(value.val6);
}

void Struct5::Read(Struct5& value, SourceStream& buffer)
{
    buffer.Read(value.val0)
        .Read(value.val1)
        .Read(value.val2)
        .Read(value.val3)
        .Read(value.val4)
        .Read(value.val5)
        .Read(value.val6);
}

DEFINE_WRITER_READER(Struct5, Struct5::Write, Struct5::Read)


void Struct6::Write(const Struct6& value, SinkStream& buffer)
{
    buffer.Write(value.mountUid)
        .Write(value.val1)
        .Write(value.val2);
}

void Struct6::Read(Struct6& value, SourceStream& buffer)
{
    buffer.Read(value.mountUid)
        .Read(value.val1)
        .Read(value.val2);
}

DEFINE_WRITER_READER(Struct6, Struct6::Write, Struct6::Read)


void Struct7::Write(const Struct7& value, SinkStream& buffer)
{
    buffer.Write(value.val0)
        .Write(value.val1)
        .Write(value.val2)
        .Write(value.val3);
}

void Struct7::Read(Struct7& value, SourceStream& buffer)
{
    buffer.Read(value.val0)
        .Read(value.val1)
        .Read(value.val2)
        .Read(value.val3);
}

DEFINE_WRITER_READER(Struct7, Struct7::Write, Struct7::Read)


void RanchHorse::Write(const RanchHorse& value, SinkStream& buffer)
{
    buffer.Write(value.ranchIndex)
        .Write(value.horse);
}

void RanchHorse::Read(RanchHorse& value, SourceStream& buffer)
{
    buffer.Read(value.ranchIndex)
        .Read(value.horse);
}

DEFINE_WRITER_READER(RanchHorse, RanchHorse::Write, RanchHorse::Read)


void RanchPlayer::Write(const RanchPlayer& value, SinkStream& buffer)
{
    buffer.Write(value.userUid)
        .Write(value.name)
        .Write(static_cast<uint8_t>(value.gender))
        .Write(value.unk0)
        .Write(value.unk1)
        .Write(value.description);

    buffer.Write(value.character)
        .Write(value.horse);

    buffer.Write(static_cast<uint8_t>(value.characterEquipment.size()));
    for (const auto& item : value.characterEquipment)
    {
        buffer.Write(item);
    }

    buffer.Write(value.playerRelatedThing);

    buffer.Write(value.ranchIndex)
        .Write(value.unk2)
        .Write(value.unk3);

    buffer.Write(value.anotherPlayerRelatedThing)
        .Write(value.yetAnotherPlayerRelatedThing);

    buffer.Write(value.unk4)
        .Write(value.unk5);
}

void RanchPlayer::Read(RanchPlayer& value, SourceStream& buffer)
{
    buffer.Read(value.userUid)
        .Read(value.name)
        .Read(reinterpret_cast<uint8_t&>(value.gender))
        .Read(value.unk0)
        .Read(value.unk1)
        .Read(value.description);

    buffer.Read(value.character)
        .Read(value.horse);

    uint8_t size;
    buffer.Read(size);
    value.characterEquipment.resize(size);
    for (auto& item : value.characterEquipment)
    {
        buffer.Read(item);
    }

    buffer.Read(value.playerRelatedThing);

    buffer.Read(value.ranchIndex)
        .Read(value.unk2)
        .Read(value.unk3);

    buffer.Read(value.anotherPlayerRelatedThing)
        .Read(value.yetAnotherPlayerRelatedThing);

    buffer.Read(value.unk4)
        .Read(value.unk5);
}

DEFINE_WRITER_READER(RanchPlayer, RanchPlayer::Write, RanchPlayer::Read)


void Quest::Write(const Quest& value, SinkStream& buffer)
{
    buffer.Write(value.unk0)
        .Write(value.unk1)
        .Write(value.unk2)
        .Write(value.unk3)
        .Write(value.unk4);
}

void Quest::Read(Quest& value, SourceStream& buffer)
{
    buffer.Read(value.unk0)
        .Read(value.unk1)
        .Read(value.unk2)
        .Read(value.unk3)
        .Read(value.unk4);
}

DEFINE_WRITER_READER(Quest, Quest::Write, Quest::Read)


} // namespace alicia