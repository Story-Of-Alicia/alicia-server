//
// Created by rgnter on 18/09/2024.
//

#ifndef COMMAND_PROTOCOL_HPP
#define COMMAND_PROTOCOL_HPP

#include "proto/LobbyMessageDefines.hpp"
#include "proto/RanchMessageDefines.hpp"

namespace alicia
{

//! A constant buffer size for message magic.
//! The maximum size of message payload is 4092 bytes.
//! The extra 4 bytes are reserved for message magic.
constexpr uint16_t BufferSize = 4096;

//! A constant buffer jumbo for message magic.
constexpr uint16_t BufferJumbo = 16384;

//! A constant 4-byte XOR control value,
//! with which message bytes are XORed.
constexpr std::array xor_control{
  static_cast<uint8_t>(0xCB),
  static_cast<uint8_t>(0x91),
  static_cast<uint8_t>(0x01),
  static_cast<uint8_t>(0xA2),
};

//! Message magic with which all messages are prefixed.
struct MessageMagic
{
  //! An ID of the message.
  uint16_t id{0};
  //! A length of message payload.
  //! Maximum payload length is 4092 bytes.
  uint16_t length{0};
};

//! Decode message magic value.
//!
//! @param value Magic value.
//! @return Decoded message magic.
MessageMagic decode_message_magic(uint32_t value);

//! Encode message magic.
//!
//! @param magic Message magic.
//! @return Encoded message magic value.
uint32_t encode_message_magic(MessageMagic magic);

//! Appy XORcodec to a buffer.
//!
//! @param buffer Buffer.
//! @returns XORcoded buffer.
template <typename Buffer> void xor_codec_cpp(Buffer &buffer)
{
  for (std::size_t idx = 0; idx < buffer.size(); idx++)
  {
    const auto shift = idx % 4;
    buffer[idx] ^= xor_control[shift];
  }
}

//! IDs of the commands in the protocol.
enum class CommandId
  : uint16_t
{
  LobbyLogin = 0x0007,
  LobbyLoginOK = 0x0008,
  LobbyLoginCancel = 0x0009,

  LobbyShowInventory = 0x007e,
  LobbyShowInventoryOK = 0x007f,
  LobbyShowInventoryCancel = 0x0080,

<<<<<<< HEAD
  LobbyAchievementCompleteList = 0x00e5,
  LobbyAchievementCompleteListOK = 0x00e6,
  LobbyAchievementCompleteListCancel = 0x00e7,

  LobbyRequestDailyQuestList = 0x0356,
  LobbyRequestDailyQuestListOK = 0x0357,
  // ToDo: Not sure about the cancel response being available.
  LobbyRequestDailyQuestListCancel = 0x0358,

  LobbyRequestLeagueInfo = 0x0376,
  LobbyRequestLeagueInfoOK = 0x0377,
  LobbyRequestLeagueInfoCancel = 0x0378,

  LobbyRequestQuestList = 0x03f8,
  LobbyRequestQuestListOK = 0x03f9,
  LobbyRequestQuestListCancel = 0x03fa,

=======
>>>>>>> parent of d81bd53 (show inventory impl)
  Count = 0xFFFF
};

//! Get the name of the command from its ID.
//! @param command ID of the command to retrieve the name for.
//! @returns If command is registered, name of the command.
//!          Otherwise, returns "n/a".
std::string_view GetCommandName(CommandId command);

} // namespace alicia


#endif //COMMAND_PROTOCOL_HPP
