//
// Created by rgnter on 3/07/2025.
//

#ifndef HORSEREGISTRY_HPP
#define HORSEREGISTRY_HPP

#include <random>
#include <unordered_map>

#include "libserver/data/DataDefinitions.hpp"

namespace server
{

struct Coat
{
  data::Tid tid{data::InvalidTid};
  //! -1 and 0
  int32_t faceType{0};
};

struct Face
{
  data::Tid tid{data::InvalidTid};
  int32_t type{0};
};

struct Mane
{
  data::Tid tid{data::InvalidTid};
  int32_t colorGroup{0};
};

struct Tail
{
  data::Tid tid{data::InvalidTid};
  int32_t colorGroup{0};
};


class HorseRegistry
{
public:
  HorseRegistry();

  void ReadConfig();

  void BuildRandomHorse(
    data::Horse::Parts& parts,
    data::Horse::Appearance& appearance);

  static HorseRegistry& Get()
  {
    static HorseRegistry instance{};
    return instance;
  }
private:

  std::random_device _randomDevice;
  std::unordered_map<data::Tid, Coat> _coats;
  std::unordered_map<data::Tid, Face> _faces;

  std::unordered_map<data::Tid, Mane> _manes;
  std::unordered_map<data::Tid, Tail> _tails;

  struct ManeTailColorGroup
  {
    data::Tid maneTid{data::InvalidTid};
    data::Tid tailTid{data::InvalidTid};
  };
  //! A vector of manes and tails with a matching color group.
  std::vector<ManeTailColorGroup> maneTailColorGroups;
};

} // namespace server

#endif //HORSEREGISTRY_HPP
