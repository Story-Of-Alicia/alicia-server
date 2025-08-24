//
// Created by rgnter on 24/08/2025.
//

#ifndef CHATTERPROTOCOL_HPP
#define CHATTERPROTOCOL_HPP

namespace alicia::protocol
{

enum class ChatterCommand
{
  ChatCmdLogin = 1,
  ChatCmdLoginAckOK = 2,
  ChatCmdLoginAckCancel = 3
};

} // namespace alicia::protocol

#endif //CHATTERPROTOCOL_HPP
