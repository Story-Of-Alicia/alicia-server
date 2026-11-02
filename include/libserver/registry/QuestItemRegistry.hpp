/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#ifndef QUEST_ITEM_REGISTRY_HPP
#define QUEST_ITEM_REGISTRY_HPP

#include <libserver/registry/Registry.hpp>

#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct QuestItemSpawnPoint
{
  uint32_t mapBlockId{};
  uint32_t deckId{};
};

struct QuestItemDeck
{
  //! Client QuestDeckID.
  uint32_t qDeckId{};
  //! Client QuestItemID. Matches the quest function value.
  uint32_t qTemId{};
  uint32_t spawnCnt{1};
  //! Candidate map block + deckId pairs this deck can spawn at.
  std::vector<QuestItemSpawnPoint> spawnPoints{};
};

class QuestItemRegistry : public Registry
{
public:
  void ReadConfig(const std::filesystem::path& configPath) override;
  void Clear() override;

  [[nodiscard]] const std::unordered_map<uint32_t, QuestItemDeck>& GetQuestItemDecks() const;

private:
  std::unordered_map<uint32_t, QuestItemDeck> _questItemDecks{};
};

} // namespace server::registry

#endif // QUEST_ITEM_REGISTRY_HPP
