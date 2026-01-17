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

#ifndef MODERATIONSYSTEM_HPP
#define MODERATIONSYSTEM_HPP

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

namespace server
{

class ModerationSystem
{
public:
  struct Verdict
  {
    //! A flag indicating whether the input should be prevented.
    bool isPrevented = false;
  };

  void ReadConfig(const std::filesystem::path& configPath);

  //! Moderates an input message and presents a verdict.
  //! @param input Message to validate.
  //! @return Moderation verdict.
  [[nodiscard]] Verdict Moderate(const std::string& input) const noexcept;

private:
  struct Word
  {
    //! A flag indicating whether the word is prevented.
    bool isPrevented = false;
    //! A regex matching the word expression.
    std::regex regex;
  };

  //! A collection of word match regexes.
  std::vector<Word> _words;
};

} // namespace server

#endif // MODERATIONSYSTEM_HPP
