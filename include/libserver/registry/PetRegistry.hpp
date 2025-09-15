//
// Created by xen1i on 27/08/2025.
//

#ifndef PETREGISTRY_HPP
#define PETREGISTRY_HPP

#include <unordered_map>

#include "libserver/data/DataDefinitions.hpp"

namespace server
{
namespace registry
{

struct Egg
{
  //! An item representing the egg.
  server::data::Tid itemTid;
  //! A time duration it takes the egg to hatch.
  server::data::Clock::duration hatchDuration;
  //! A vector of pets that can hatch from the egg.
  std::vector<server::data::Tid> hatchablePets;
};

struct Pet
{
  server::data::Tid petTid{}; // petItem tid
  uint32_t petId{}; // pet id for spawning
};

class PetRegistry final
{
public:
  PetRegistry();

  void ReadConfig();

  Egg GetEgg(server::data::Tid tid);

  Pet GetPet(server::data::Tid tid);

  static PetRegistry& GetInstance()
  {
    static PetRegistry instance{};
    return instance;
  }

private:
  std::unordered_map<server::data::Tid, Egg> _eggs;
  std::unordered_map<server::data::Tid, Pet> _pets;
};


} // namespace registry

} // namespace server

#endif // PETREGISTRY_HPP
