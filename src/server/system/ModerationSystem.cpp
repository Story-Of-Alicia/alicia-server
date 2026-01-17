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

#include "server/system/ModerationSystem.hpp"

#include <yaml-cpp/yaml.h>

namespace server
{

void ModerationSystem::ReadConfig(
  const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto wordsSection = root["words"];
  if (not wordsSection)
    throw std::runtime_error("Missing words section");

  const auto wordsCollectionSection = wordsSection["collection"];
  if (not wordsCollectionSection)
    throw std::runtime_error("Missing words collection section");

  for (const auto& wordSection : wordsCollectionSection)
  {
    const auto word = wordSection["word"].as<std::string>();
    const bool isPrevented = wordSection["prevent"].as<bool>(false);

    _words.emplace_back(
      Word{
        .isPrevented = isPrevented,
        .regex = std::regex(word, std::regex_constants::ECMAScript | std::regex_constants::icase)});
  }
}

ModerationSystem::Verdict ModerationSystem::Moderate(
  const std::string& input) const noexcept
{
  Verdict verdict;

  for (const auto& word : _words)
  {
    // Check if any part of the input matches the word.
    if (!std::regex_match(input, word.regex))
      continue;

    // Check if the word is prevented or just censored.
    if (!word.isPrevented)
      continue;

    // todo: censor

    verdict.isPrevented = true;
    break;
  }

  return verdict;
}

} // namespace server

