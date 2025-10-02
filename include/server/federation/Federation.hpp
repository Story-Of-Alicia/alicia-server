//
// Created by rgnter on 30/09/2025.
//

#ifndef FEDERATION_HPP
#define FEDERATION_HPP

#include "boost/beast.hpp"

namespace server::federation
{

class Federation
{
public:
  void Initialize();
  void Terminate();
};

} // namespace server::federation


#endif //FEDERATION_HPP
