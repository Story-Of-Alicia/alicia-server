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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <boost/asio.hpp>
#include <tinyxml2.h>

#include <chrono>
#include <span>

namespace server::util
{

namespace asio = boost::asio;

using Clock = std::chrono::system_clock;

//! Windows file-time represents number of 100 nanosecond intervals since January 1, 1601 (UTC).
struct WinFileTime
{
  uint32_t dwLowDateTime = 0;
  uint32_t dwHighDateTime = 0;
};

//! A zero-cost struct to represent a date and a time.
struct DateTime
{
  int32_t years = 0;
  uint32_t months = 0;
  uint32_t days = 0;
  int32_t hours = 0;
  int32_t minutes = 0;
  int32_t seconds = 0;
};

struct ShopList
{
  struct Goods
  {
    //! Goods sequence number (internal unique shop item number, incremental, cannot be 0)
    uint32_t goodsSq{};
    //! 0 - Goods info | 1 - Set (package)
    uint32_t setType{};
    //! Currency to pay with for the goods
    enum class MoneyType
    {
      Carrots = 0,
      Cash = 1
    } moneyType{MoneyType::Carrots};
    //! Item offer type
    enum class GoodsType
    {
      Default = 0,
      New = 1,
      Limited = 2,
      Sale = 3,
      PCBang = 4
    } goodsType{GoodsType::Default};
    // TODO: recommended category?
    uint32_t recommendType{};
    //! 1 - Do not show in suggested/recommended tab
    uint32_t recommendNo{};
    //! Can character gift shop item to another character
    enum class GiftType
    {
      NoGifting = 0,
      CanGift = 1
    } giftType{GiftType::NoGifting};
    //! "Best top 5" ordering 1 <= rank <= 5
    uint32_t salesRank{};
    //! Bonus on purchase
    uint32_t bonusGameMoney{};
    //! Item name (TODO: does it need to be wrapped in CDATA?)
    std::string goodsNm{};
    //! Item description
    std::string goodsDesc{};
    std::string itemCapacityDesc{};
    //! 1 - Shows item in shop, anything else hides it
    uint32_t sellSt{};
    //! Item TID
    uint32_t itemUid{};
    //! Only valid when setType = 1
    uint32_t setPrice{};

    struct Item
    {
      //! Unique price ID for the offer, must not be 0
      uint32_t priceId{};
      //! Item count for the price
      uint32_t priceRange{};
      //! Item price
      //! Only valid when setType = 0
      uint32_t goodsPrice{};
      //! Only valid when setType = 1
      uint32_t itemUid{};
    };
    std::vector<Item> items{};
  };
  std::vector<Goods> goodsList{};
};

//! Converts a time point to the Windows file time.
//! @param timePoint Point in time.
//! @return Windows file time representing specified point in time.
WinFileTime TimePointToFileTime(const Clock::time_point& timePoint);

//! Converts date time to alicia time.
//! @param dateTime Date and time.
//! @returns Alicia time representing the date and time.
uint32_t DateTimeToAliciaTime(const DateTime& dateTime);

//! Converts time point to alicia time.
//! @param timePoint Time point.
//! @returns Alicia time representing the date and time of the time point.
uint32_t TimePointToAliciaTime(const Clock::time_point& timePoint);

//! Converts duration to alicia time.
//! @param duration Duration.
//! @returns Alicia time representing the duration.
uint32_t DurationToAliciaTime(const Clock::duration& duration);

/// @brief Converts Alicia shop timestamp to DateTime
/// @param timestamp Alicia shop timestamp
/// @return Date time representation
DateTime AliciaShopTimeToDateTime(const std::array<uint32_t, 3> timestamp);

/// @brief Converts DateTime to Alicia shop timestamp
/// @param dateTime Date time
/// @return Alicia shop timestamp representing the date and time
std::array<uint32_t, 3> DateTimeToAliciaShopTime(const DateTime& dateTime);

/// @brief Converts Alicia shop timestamp to time point
/// @param timestamp Alicia shop timestamp
/// @return Time point representing the date and time
Clock::time_point AliciaShopTimeToTimePoint(const std::array<uint32_t, 3>& timestamp);

/// @brief Converts time point to Alicia shop timestamp
/// @param timestamp Time stamp
/// @return Alicia shop timestamp representing the date and time
std::array<uint32_t, 3> TimePointToAliciaShopTime(const Clock::time_point& timestamp);

asio::ip::address_v4 ResolveHostName(const std::string& host);

std::string GenerateByteDump(std::span<const std::byte> data);

std::vector<std::string> TokenizeString(const std::string& value, char delimiter);

ShopList GetSampleShopList();

std::string ShopListToXmlString(const ShopList& shopList);

} // namespace server::util

#endif // UTIL_HPP
