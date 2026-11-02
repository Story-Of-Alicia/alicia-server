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

#include "libserver/registry/QuestItemRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace server::registry
{

namespace
{

void ReadQuestItemDeck(QuestItemDeck& deck, const YAML::Node& yaml)
{
  deck.qDeckId = yaml["qDeckId"].as<decltype(QuestItemDeck::qDeckId)>(0);
  deck.qTemId = yaml["qTemId"].as<decltype(QuestItemDeck::qTemId)>(0);
  deck.spawnCnt = yaml["spawnCnt"].as<decltype(QuestItemDeck::spawnCnt)>(1);

  if (const auto spawnPointsNode = yaml["spawnPoints"])
  {
    for (const auto& spawnPointNode : spawnPointsNode)
    {
      QuestItemSpawnPoint spawnPoint{};
      spawnPoint.mapBlockId = spawnPointNode["mapBlockId"].as<uint32_t>(0);
      spawnPoint.deckId = spawnPointNode["deckId"].as<uint32_t>(0);
      deck.spawnPoints.push_back(spawnPoint);
    }
  }
}

} // anonymous namespace

void QuestItemRegistry::Clear()
{
  _questItemDecks.clear();
}

void QuestItemRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto decksSection = root["questItemDecks"];
  if (not decksSection)
    throw std::runtime_error("Missing questItemDecks section");

  const auto collectionSection = decksSection["collection"];
  if (not collectionSection)
    throw std::runtime_error("Missing questItemDecks.collection section");

  Clear();

  for (const auto& deckNode : collectionSection)
  {
    QuestItemDeck deck{};
    ReadQuestItemDeck(deck, deckNode);
    _questItemDecks.try_emplace(deck.qDeckId, deck);
  }

  spdlog::info("Quest item registry loaded {} quest item decks", _questItemDecks.size());
}

const std::unordered_map<uint32_t, QuestItemDeck>& QuestItemRegistry::GetQuestItemDecks() const
{
  return _questItemDecks;
}

} // namespace server::registry
