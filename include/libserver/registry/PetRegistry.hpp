//
// Created by xen1i on 27/08/2025.
//

#ifndef PETREGISTRY_HPP
#define PETREGISTRY_HPP

#include "libserver/data/DataDefinitions.hpp"

namespace server
{
struct PetItem
{
  uint32_t eggTid{};
  uint32_t petTid{};
  uint32_t petId{};
};

class PetRegistry
{
public:
  PetRegistry();

  void ReadConfig();

  std::vector<std::pair<uint32_t, uint32_t>> BuildHatchTable(
    uint32_t eggTid);

  static PetRegistry& Get()
  {
    static PetRegistry instance{};
    return instance;
  }
private:
  
  std::vector<PetItem> _pets;
};


} // namespace server

#endif // PETREGISTRY_HPP
