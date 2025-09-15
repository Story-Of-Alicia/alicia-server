//
// Created by rgnter on 14/09/2025.
//

#ifndef ITEMREGISTRY_HPP
#define ITEMREGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace server
{

namespace registry
{

struct Item
{
  struct CharacterPartInfo
  {
    uint32_t characterId{};
    enum class Slot
    {
      None = 0,
      Hat = 1,
      Head = 2,
      Body = 16,
      Legs = 32,
      Suit = Body & Legs,
      Earrings = 64,
    } slot{Slot::None};
  };

  struct MountPartInfo
  {
    enum class Slot
    {
      None = 0,
      Saddle = 1,
      Horseshoe = 8,
      Protector = 16,
      Shield = 32,
    } slot{Slot::None};
  };

  struct MountPartSetInfo
  {
    uint32_t skinId{};
    uint32_t maneId{};
    uint32_t tailId{};
    uint32_t faceId{};
    uint32_t scale{};
    uint32_t legLength{};
    uint32_t legVolume{};
    uint32_t bodyLength{};
    uint32_t bodyVolume{};
    uint32_t emblemId{};
  };

  struct CareParameters
  {
    uint32_t cleanPoints{};
    uint32_t polishPoints{};
    enum class Part
    {
      Body = 0,
      Mane = 1,
      Tail = 2
    } parts;
  };

  struct CureParameters
  {
    uint32_t injury{};
  };

  struct FoodParameters
  {
    enum class FeedType
    {
      Unknown1 = 0,
      Unknown2 = 1
    } feedType{FeedType::Unknown1};
    uint32_t friendlinessPoints{};
    uint32_t plenitudePoints{};
    uint32_t preferenceType{};
  };

  struct PlayParameters
  {
    uint32_t friendlinessPoints{};
    uint32_t friendlinessPointsOnFailure{};
    uint32_t charmPoints{};
    uint32_t charmPointsOnFailure{};
    uint32_t minAttachment{};
    uint32_t maxAttachment{};
  };

  uint32_t tid{};
  std::string name;
  std::string description;
  uint32_t level{};

  std::optional<CharacterPartInfo> characterPartInfo;
  std::optional<MountPartInfo> mountPartInfo;
  std::optional<MountPartSetInfo> mountPartSetInfo;

  std::optional<CareParameters> careParameters;
  std::optional<CureParameters> cureParameters;
  std::optional<FoodParameters> foodParameters;
  std::optional<PlayParameters> playParameters;
};

class ItemRegistry
{
public:
  void ReadConfig(const std::filesystem::path& configPath);
  [[nodisacrd]] std::optional<Item> GetItem(uint32_t tid);

private:
  std::unordered_map<uint32_t, Item> _items;
};

} // namespace registry

} // namespace server

#endif //ITEMREGISTRY_HPP
