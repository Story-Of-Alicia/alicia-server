//
// Created by rgnter on 8/09/2024.
//

#ifndef PROTO_HPP
#define PROTO_HPP

#include <array>
#include <cstdint>

namespace alicia
{

//!
enum class CommandId
{
  LobbyCommandLogin = 0x0007,
  LobbyCommandLoginOK = 0x0008,
  LobbyCommandLoginCancel = 0x0009,

  LobbyShowInventory = 0x007e,
  LobbyShowInventoryOK = 0x007f,
  LobbyShowInventoryCancel = 0x0080,
};

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

//! Message magix with which all messages are prefixed.
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
  for (size_t idx = 0; idx < buffer.size(); idx++)
  {
    const auto shift = idx % 4;
    buffer[idx] ^= xor_control[shift];
  }
}

} // namespace alicia

#endif //PROTO_HPP