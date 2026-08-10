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

#include "libserver/data/DataRepair.hpp"

#include "libserver/data/DataDirector.hpp"
#include "libserver/registry/HorseRegistry.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <string_view>
#include <type_traits>

namespace server
{

namespace
{

//! The TID of the horse built as a replacement for a damaged mount.
//! The default brown horse, as handed out on character creation.
constexpr data::Tid ReplacementMountTid = 20001;

//! Returns whether a datum is damaged beyond repair.
//! @param storage Storage owning the datum.
//! @param uid UID of the datum.
template <typename Storage>
bool IsDamaged(Storage& storage, const data::Uid uid)
{
  if (uid == data::InvalidUid)
    return false;

  if (storage.GetRetrieveFailureCount(uid) == 0)
    return false;

  if (not storage.HasRetrieveFailedFor(uid, repair::DamagedGracePeriod))
  {
    storage.RetryRetrieve(uid);
    return false;
  }

  return true;
}

//! Returns whether any of the referenced data are damaged beyond repair.
//! @param storage Storage owning the data.
//! @param uids UIDs of the referenced data.
template <typename Storage, typename Container>
bool AreAnyDamaged(Storage& storage, const Container& uids)
{
  // Every UID is visited, as visiting also queues the retrieval retries.
  bool anyDamaged = false;
  for (const auto uid : uids)
  {
    if (IsDamaged(storage, uid))
      anyDamaged = true;
  }

  return anyDamaged;
}

//! Drops the references to damaged data from a collection of references.
//! @param storage Storage owning the data.
//! @param uids UIDs of the referenced data.
//! @param referenceName Name of the reference, used for logging.
//! @param characterUid UID of the character holding the references.
template <typename Storage, typename Container>
void DropDamagedReferences(
  Storage& storage,
  Container& uids,
  const std::string_view referenceName,
  const data::Uid characterUid)
{
  std::erase_if(
    uids,
    [&storage, referenceName, characterUid](const data::Uid uid)
    {
      if (not IsDamaged(storage, uid))
        return false;

      spdlog::error(
        "Dropping the reference to the damaged {} {} of character {}",
        referenceName,
        uid,
        characterUid);
      return true;
    });
}

//! Clears a single reference if the datum it references is damaged.
//! @param storage Storage owning the datum.
//! @param uid Reference to the datum.
//! @param referenceName Name of the reference, used for logging.
//! @param characterUid UID of the character holding the reference.
template <typename Storage>
void ClearDamagedReference(
  Storage& storage,
  dao::Field<data::Uid>& uid,
  const std::string_view referenceName,
  const data::Uid characterUid)
{
  if (not IsDamaged(storage, uid()))
    return;

  spdlog::error(
    "Dropping the reference to the damaged {} {} of character {}",
    referenceName,
    uid(),
    characterUid);
  uid = data::InvalidUid;
}

//! @param dataDirector Data director owning the data.
//! @param character Character holding the references.
//! @param handler Handler invoked as `handler(storage, referenceName, reference)`.
template <typename Character, typename Handler>
void VisitCharacterReferences(
  DataDirector& dataDirector,
  Character& character,
  Handler&& handler)
{
  handler(dataDirector.GetItemCache(), "inventory item", character.inventory());
  handler(dataDirector.GetItemCache(), "equipped item", character.characterEquipment());

  handler(dataDirector.GetHorseCache(), "horse", character.horses());
  handler(dataDirector.GetHorseCache(), "wishlisted horse", character.breedingWishlist());

  handler(dataDirector.GetEggCache(), "egg", character.eggs());
  handler(dataDirector.GetPetCache(), "pet", character.pets());
  handler(dataDirector.GetHousingCache(), "housing", character.housing());

  handler(dataDirector.GetStorageItemCache(), "gift", character.gifts());
  handler(dataDirector.GetStorageItemCache(), "purchase", character.purchases());

  handler(dataDirector.GetMailCache(), "inbox mail", character.mailbox.inbox());
  handler(dataDirector.GetMailCache(), "sent mail", character.mailbox.sent());

  handler(dataDirector.GetQuestCache(), "quest", character.quests());

  handler(dataDirector.GetGuildCache(), "guild", character.guildUid);
  handler(dataDirector.GetPetCache(), "equipped pet", character.petUid);
  handler(dataDirector.GetSettingsCache(), "settings", character.settingsUid);
  handler(
    dataDirector.GetDailyQuestGroupCache(),
    "daily quest group",
    character.dailyQuestGroupUid);
}

//! Returns whether the reference is a single UID field instead of a collection of UIDs.
template <typename Reference>
constexpr bool IsSingleReference =
  std::is_same_v<std::remove_cvref_t<Reference>, dao::Field<data::Uid>>;

} // anon namespace

bool repair::CleanseCharacterReferences(
  DataDirector& dataDirector,
  const data::Uid characterUid)
{
  const auto characterRecord = dataDirector.GetCharacter(characterUid);
  if (not characterRecord)
    return false;

  bool anyDamaged = false;
  bool isMountDamaged = false;
  auto replacementMountUid = data::InvalidUid;

  // Determine whether anything needs to be repaired at all,
  // so that the character record is not needlessly patched and stored.
  characterRecord.Immutable(
    [&dataDirector, &anyDamaged, &isMountDamaged, &replacementMountUid](
      const data::Character& character)
    {
      VisitCharacterReferences(
        dataDirector,
        character,
        [&anyDamaged](auto& storage, const std::string_view, const auto& reference)
        {
          if constexpr (IsSingleReference<decltype(reference)>)
          {
            if (IsDamaged(storage, reference()))
              anyDamaged = true;
          }
          else
          {
            if (AreAnyDamaged(storage, reference))
              anyDamaged = true;
          }
        });

      // The mount is referenced separately from the horses of the character,
      // and the character is expected to always have one.
      isMountDamaged = IsDamaged(dataDirector.GetHorseCache(), character.mountUid());
      if (not isMountDamaged)
        return;

      anyDamaged = true;

      // Prefer one of the intact horses of the character as the replacement mount.
      for (const auto horseUid : character.horses())
      {
        if (IsDamaged(dataDirector.GetHorseCache(), horseUid))
          continue;

        replacementMountUid = horseUid;
        break;
      }
    });

  if (not anyDamaged)
    return false;

  // Every horse of the character is damaged too, so a new mount has to be built.
  if (isMountDamaged && replacementMountUid == data::InvalidUid)
  {
    const auto mountRecord = dataDirector.CreateHorse();
    if (mountRecord)
    {
      mountRecord.Mutable(
        [&replacementMountUid](data::Horse& horse)
        {
          registry::HorseRegistry::BuildDefaultHorse(horse, ReplacementMountTid);
          replacementMountUid = horse.uid();
        });
    }
  }

  characterRecord.Mutable(
    [&dataDirector, characterUid, isMountDamaged, replacementMountUid](
      data::Character& character)
    {
      VisitCharacterReferences(
        dataDirector,
        character,
        [characterUid](auto& storage, const std::string_view referenceName, auto& reference)
        {
          if constexpr (IsSingleReference<decltype(reference)>)
            ClearDamagedReference(storage, reference, referenceName, characterUid);
          else
            DropDamagedReferences(storage, reference, referenceName, characterUid);
        });

      if (not isMountDamaged)
        return;

      // A character without a mount can not be loaded either,
      // so leave the damaged mount in place rather than make the character worse off.
      if (replacementMountUid == data::InvalidUid)
      {
        spdlog::error(
          "Character {} has the damaged mount {}, for which no replacement could be built",
          characterUid,
          character.mountUid());
        return;
      }

      spdlog::error(
        "Dropping the reference to the damaged mount {} of character {}, replaced by mount {}",
        character.mountUid(),
        characterUid,
        replacementMountUid);

      character.mountUid = replacementMountUid;
      // The replacement mount is no longer one of the plain horses of the character.
      std::erase(character.horses(), replacementMountUid);
    });

  return true;
}

bool repair::CleanseUserReferences(
  DataDirector& dataDirector,
  const std::string& userName)
{
  const auto userRecord = dataDirector.GetUser(userName);
  if (not userRecord)
    return false;

  bool anyDamaged = false;
  userRecord.Immutable(
    [&dataDirector, &anyDamaged](const data::User& user)
    {
      anyDamaged = AreAnyDamaged(dataDirector.GetInfractionCache(), user.infractions());
    });

  if (not anyDamaged)
    return false;

  userRecord.Mutable(
    [&dataDirector, &userName](data::User& user)
    {
      std::erase_if(
        user.infractions(),
        [&dataDirector, &userName](const data::Uid uid)
        {
          if (not IsDamaged(dataDirector.GetInfractionCache(), uid))
            return false;

          spdlog::error(
            "Dropping the reference to the damaged infraction {} of user '{}'",
            uid,
            userName);
          return true;
        });
    });

  return true;
}

} // namespace server
