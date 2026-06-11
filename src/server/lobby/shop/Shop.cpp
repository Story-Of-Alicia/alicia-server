//
// Created by maros.prejsa on 30/10/2025.
//

#include "server/lobby/shop/Shop.hpp"

#include "libserver/registry/ItemRegistry.hpp"

#include <tinyxml2.h>
#include <algorithm>
#include <vector>

namespace server
{

namespace
{

std::string ShopListToXmlString(const ShopList& shopList)
{
  tinyxml2::XMLDocument doc;
  doc.InsertFirstChild(
    doc.NewDeclaration(R"(xml version="1.0" encoding="euc-kr")"));

  // Begin <ShopList>
  const auto& shopListElem = doc.NewElement("ShopList");
  doc.InsertEndChild(shopListElem);

  // Iterate goods
  for (const auto& [goodsSq, goods] : shopList.goodsList)
  {
    // Begin <GoodsList>
    const auto& goodsElem = doc.NewElement("GoodsList");
    goodsElem->InsertNewChildElement("GoodsSQ")->SetText(goods.goodsSq);
    goodsElem->InsertNewChildElement("SetType")->SetText(goods.setType);
    goodsElem->InsertNewChildElement("MoneyType")->SetText(static_cast<uint32_t>(goods.moneyType));
    goodsElem->InsertNewChildElement("GoodsType")->SetText(static_cast<uint32_t>(goods.goodsType));
    goodsElem->InsertNewChildElement("RecommendType")->SetText(goods.recommendType);
    goodsElem->InsertNewChildElement("RecommendNO")->SetText(goods.recommendNo);
    goodsElem->InsertNewChildElement("GiftType")->SetText(static_cast<uint32_t>(goods.giftType));
    goodsElem->InsertNewChildElement("SalesRank")->SetText(goods.salesRank);
    goodsElem->InsertNewChildElement("BonusGameMoney")->SetText(goods.bonusGameMoney);
    goodsElem->InsertNewChildElement("GoodsNM")->SetText(goods.goodsNm.c_str());
    goodsElem->InsertNewChildElement("GoodsDesc")->SetText(goods.goodsDesc.c_str());
    goodsElem->InsertNewChildElement("ItemCapacityDesc")->SetText(goods.itemCapacityDesc.c_str());
    goodsElem->InsertNewChildElement("SellST")->SetText(goods.sellSt);
    goodsElem->InsertNewChildElement("ItemUID")->SetText(goods.itemUid);
    if (goods.setType == 1)
      goodsElem->InsertNewChildElement("SetPrice")->SetText(goods.setPrice);

    // Begin <ItemElem>
    const auto& itemElem = doc.NewElement("ItemElem");
    for (size_t i = 0; i < goods.items.size(); ++i)
    {
      const auto& item = goods.items[i];

      // Begin <Item>
      const auto& itemXmlElem = doc.NewElement("Item");
      itemXmlElem->InsertNewChildElement("PriceID")->SetText(item.priceId);
      itemXmlElem->InsertNewChildElement("PriceRange")->SetText(item.priceRange);
      if (goods.setType == 0)
        itemXmlElem->InsertNewChildElement("GoodsPrice")->SetText(item.goodsPrice);
      else
        itemXmlElem->InsertNewChildElement("ItemUID")->SetText(item.itemUid);
      // End <Item>
      itemElem->InsertEndChild(itemXmlElem);
    }
    // End <ItemElem>
    goodsElem->InsertEndChild(itemElem);

    // End <GoodsList> element
    shopListElem->InsertEndChild(goodsElem);
  }

  // Enable XML compact mode
  constexpr bool compact = true;
  tinyxml2::XMLPrinter printer(nullptr, compact);
  doc.Print(&printer);

  //! Re-encode string into EUC-KR to show the item name correctly in KR (if any).
  return locale::FromUtf8(printer.CStr());
}

} // namespace

void ShopManager::GenerateShopList(registry::ItemRegistry& itemRegistry)
{
  uint32_t goodsSequenceId = 0;
  uint32_t recommendNoId = 0;

  auto items = itemRegistry.GetItems();
  std::vector<std::pair<uint32_t, registry::Item>> sortedItems(items.begin(), items.end());
  // Sort by unlock level ascending, then by TID as a tiebreaker.
  std::sort(sortedItems.begin(), sortedItems.end(),
    [](const auto& first, const auto& second)
    {
      const auto& [firstTid, firstItem] = first;
      const auto& [secondTid, secondItem] = second;
      if (firstItem.level != secondItem.level)
        return firstItem.level < secondItem.level;
      return firstTid < secondTid;
    });

  for (const auto& [tid, item] : sortedItems)
  {
    ++goodsSequenceId;

    if (not item.shopInfo || not item.shopInfo->isPurchasable)
      continue;

    // Pets (3/6) and eggs (3/7) must not appear in the shop
    if (item.itemIndex.category == 3 && (item.itemIndex.subcategory == 6 || item.itemIndex.subcategory == 7))
      continue;

    const auto& shopInfo = item.shopInfo.value();
    const bool isTemporary = item.type == registry::Item::Type::Temporary;
    const uint32_t recommendNo = isTemporary ? ++recommendNoId : 0u;

    ShopList::Goods goods{
      .goodsSq = goodsSequenceId,
      .setType = 0,
      .moneyType = static_cast<ShopList::Goods::MoneyType>(
        static_cast<uint32_t>(shopInfo.moneyType)),
      .goodsType = ShopList::Goods::GoodsType::Default,
      .recommendType = isTemporary ? 1u : 0u,
      .recommendNo = recommendNo,
      .giftType = item.characterPartInfo
                    ? ShopList::Goods::GiftType::CanGift
                    : ShopList::Goods::GiftType::NoGifting,
      .salesRank = 0,
      .bonusGameMoney = 0,
      .goodsNm = item.name,
      .goodsDesc = "",
      .itemCapacityDesc = "Item Capacity Description Something",
      .sellSt = 1,
      .itemUid = tid};

    uint32_t priceId = 0;
    for (const auto& priceRange : shopInfo.priceRanges)
    {
      goods.items.push_back(ShopList::Goods::Item{
        .priceId = ++priceId,
        .priceRange = priceRange.range,
        .goodsPrice = priceRange.price});
    }

    _shopList.goodsList.emplace(goodsSequenceId, goods);
  }
}

ShopList& ShopManager::GetShopList()
{
  return _shopList;
}

const std::string& ShopManager::GetSerializedShopList()
{
  if (_serializedShopList.empty())
  {
    _serializedShopList = ShopListToXmlString(_shopList);
  }

  return _serializedShopList;
}

} // namespace server
