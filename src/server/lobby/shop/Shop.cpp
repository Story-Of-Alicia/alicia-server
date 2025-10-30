//
// Created by maros.prejsa on 30/10/2025.
//

#include "server/lobby/shop/Shop.hpp"

#include "libserver/registry/ItemRegistry.hpp"

#include <tinyxml2.h>

namespace server
{

namespace
{

std::string ShopListToXmlString(const ShopList& shopList)
{
  tinyxml2::XMLDocument doc;
  doc.InsertFirstChild(
    doc.NewDeclaration(R"(xml version="1.0" encoding="utf-8")"));

  // Begin <ShopList>
  const auto& shopListElem = doc.NewElement("ShopList");
  doc.InsertEndChild(shopListElem);

  // Iterate goods
  for (auto g = 0; g < shopList.goodsList.size(); ++g)
  {
    // Get goods entry
    const auto& goods = shopList.goodsList[g];

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
    for (auto i = 0; i < goods.items.size(); ++i)
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

  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);

  return printer.CStr();
}

}

void ShopManager::GenerateShopList(registry::ItemRegistry& itemRegistry)
{
  uint32_t goodsSequenceId = 0;
  for (const auto& [tid, item] : itemRegistry.GetItems())
  {
    ++goodsSequenceId;

    if (item.careParameters || item.cureParameters || item.foodParameters || item.playParameters)
    {
      _shopList.goodsList.emplace_back(
        ShopList::Goods{
          .goodsSq = goodsSequenceId,
          .setType = 0,
          .moneyType = ShopList::Goods::MoneyType::Carrots,
          .goodsType = ShopList::Goods::GoodsType::Default,
          .recommendType = 0,
          .recommendNo = 0,
          .giftType = ShopList::Goods::GiftType::NoGifting,
          .salesRank = 0,
          .bonusGameMoney = 0,
          .goodsNm = item.name,
          .goodsDesc = "care item",
          .itemCapacityDesc = "Item Capacity Description Something",
          .sellSt = 1,
          .itemUid = tid,
          .items = {
            ShopList::Goods::Item{
              .priceId = 1,
              .priceRange = 1,
              .goodsPrice = 1},
            ShopList::Goods::Item{
              .priceId = 2,
              .priceRange = 10,
              .goodsPrice = 10},
            ShopList::Goods::Item{
              .priceId = 3,
              .priceRange = 100,
              .goodsPrice = 100}}});
    }

    if (item.characterPartInfo)
    {
      _shopList.goodsList.emplace_back(
        ShopList::Goods{
          .goodsSq = goodsSequenceId,
          .setType = 0,
          .moneyType = ShopList::Goods::MoneyType::Carrots,
          .goodsType = ShopList::Goods::GoodsType::New,
          .recommendType = 1,
          .recommendNo = 1,
          .giftType = ShopList::Goods::GiftType::NoGifting,
          .salesRank = 0,
          .bonusGameMoney = 1000,
          .goodsNm = item.name,
          .goodsDesc = "character item",
          .itemCapacityDesc = "Item Capacity Description Something",
          .sellSt = 1,
          .itemUid = tid,
          .setPrice = 5,
          .items = {
            ShopList::Goods::Item{
              .priceId = 1,
              .priceRange = 1,
              .itemUid = tid}}});
    }
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