//
// Created by maros.prejsa on 30/10/2025.
//

#ifndef ALICIA_SERVER_SHOP_HPP
#define ALICIA_SERVER_SHOP_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace server
{

namespace registry
{
class ItemRegistry;
}

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
    //! 1 - Shows in suggested/recommended tab.
    uint32_t recommendType{};
    //! Unique sequence number for the goods in suggested/recommended tab.
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
    //! Item description. If empty, the game client will use the description found (if any) in libconfig.
    std::string goodsDesc{};
    // Unknown
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
      //! Item count for the price or time in days (measured in hours) for temporary items
      uint32_t priceRange{};
      //! Item price
      //! Only valid when setType = 0
      uint32_t goodsPrice{};
      //! Only valid when setType = 1
      uint32_t itemUid{};
    };

    std::vector<Item> items{};
  };

  std::unordered_map<uint32_t, Goods> goodsList{};
};

class ShopManager
{
public:
  void GenerateShopList(registry::ItemRegistry& itemRegistry);
  ShopList& GetShopList();
  const std::string& GetSerializedShopList();

private:
  ShopList _shopList;
  std::string _serializedShopList;
};


} // namespace server

#endif // ALICIA_SERVER_SHOP_HPP
