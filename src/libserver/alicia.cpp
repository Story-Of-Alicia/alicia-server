#include <bit>
#include <iostream>

#include "libserver/alicia.hpp"
#include "libserver/mapping.hpp"

uint16_t MUTE_COMMAND_IDS[] = {
  #ifdef AcCmdCLHeartbeat
  AcCmdCLHeartbeat,
  #endif

  #ifdef AcCmdCRHeartbeat
  AcCmdCRHeartbeat
  #endif
};

namespace
{

} // namespace anon

void send_command(boost::asio::ip::tcp::socket& socket, alicia::ICommand& cmd)
{
  bool mute = false;
  for (auto &muteCmdId : MUTE_COMMAND_IDS)
  {
    if (cmd.GetCommandId() == muteCmdId)
    {
      mute = true;
      break;
    }
  }
  if(!mute)
  {
    std::cout << ">>> SEND " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port() << " ";
    cmd.Log();
  }

  std::vector<uint8_t> cmdContents = cmd.AsBytes();
  // gametree forgot to encode clientbound packets
  //alicia::xor_codec_cpp(cmdContents);
  uint16_t totalPacketSize = sizeof(uint32_t) + cmdContents.size(); // size of the magic header + size of the packet contents
  uint32_t responseEncodedMagic = alicia::encode_message_magic({cmd.GetCommandId(), totalPacketSize});
  socket.write_some(boost::asio::const_buffer(&responseEncodedMagic, sizeof(uint32_t)));
  socket.write_some(boost::asio::const_buffer(cmdContents.data(), cmdContents.size()));
}

alicia::MessageMagic alicia::decode_message_magic(uint32_t value)
{
  MessageMagic magic;
  if (value & 1 << 15) {
    const uint16_t section = value & 0x3FFF;
    magic.length = (value & 0xFF) << 4 | section >> 8 & 0xF | section & 0xF000;
  }

  const uint16_t firstTwoBytes = value & 0xFFFF;
  const uint16_t secondTwoBytes = value >> 16 & 0xFFFF;
  const uint16_t xorResult = firstTwoBytes ^ secondTwoBytes;
  magic.id = ~(xorResult & 0xC000) & xorResult;

  return magic;
}

uint32_t alicia::encode_message_magic(MessageMagic magic)
{
  const uint16_t id = BufferJumbo & 0xFFFF | magic.id & 0xFFFF;
  const uint32_t length = BufferSize << 16 | magic.length;

  uint32_t encoded = length;
  encoded = (encoded & 0x3FFF | encoded << 14) & 0xFFFF;
  encoded = ((encoded & 0xF | 0xFF80) << 8 | length >> 4 & 0xFF | encoded & 0xF000) & 0xFFFF;
  encoded |= (encoded ^ id) << 16;
  return encoded;
}

void alicia::read(std::istream& stream, std::string& val)
{
  while(true) {
    char v{0};
    stream.read(&v, sizeof(v));
    if(v == 0)
      break;
    val += v;
  }
}

alicia::DummyCommand::DummyCommand(uint16_t cId)
{
  this->commandId = cId;
  this->timestamp = std::chrono::system_clock::now();
}

uint16_t alicia::DummyCommand::GetCommandId()
{
  return commandId;
}

std::vector<uint8_t>& alicia::DummyCommand::AsBytes()
{
  return data;
}

void alicia::DummyCommand::Log()
{
  std::string_view commandName = GetMessageName(commandId);
  std::cout << commandName << " (0x" << std::hex << commandId << ")" << std::dec;
  
  char rowString[17];
  memset(rowString, 0, 17);

  int column = 0;
  for (int i = 0; i < data.size(); ++i) {
    column = i%16;
    switch(column)
    {
      case 0:
        printf("\t%s\n\t", rowString);
        memset(rowString, 0, 17);
        break;
      case 8:
        printf(" ");
        break;
    }

    uint8_t datum = data[i];
    if(datum >= 32 && datum <= 126) {
      rowString[column] = (char)datum;
    } else {
      rowString[column] = '.';
    }

    printf(" %02X", datum);
  }
  printf("%*s\t%s\n\n", (16-column)*3, "", rowString);
}

void alicia::Client::read_loop()
{
  _socket.async_read_some(_buffer.prepare(4096), [&](boost::system::error_code error, std::size_t size) {
    if(error) {
      printf(
          "Error occurred on read loop with client on port %d. What: %s\n",
          _socket.remote_endpoint().port(),
          error.message().c_str());
      return;
    }

    // Commit the recieved buffer.
    _buffer.commit(size);

    std::istream stream(&_buffer);

    // Read the message magic.
    uint32_t magic = 0;
    read(stream, magic);
    if(magic == 0x0) {
      throw std::runtime_error("invalid message magic");
    }

    const auto message_magic = decode_message_magic(magic);

    // Read the message data.
    std::vector<uint8_t> data;
    data.resize(message_magic.length - 4);
    read(stream, data);

    // XOR decode
    xor_codec_cpp(data);

    DummyCommand request = DummyCommand(message_magic.id);
    request.timestamp = std::chrono::system_clock::now();
    request.data = data;
    bool mute = false;
    for (auto &muteCmdId : MUTE_COMMAND_IDS)
    {
      if (message_magic.id == muteCmdId)
      {
        mute = true;
        break;
      }
    }
    if(!mute)
    {
      std::cout << "<<< RECV " << _socket.remote_endpoint().address().to_string() << ":" << _socket.remote_endpoint().port() << " ";
      request.Log();
    }
    
    switch(message_magic.id)
    {
      #ifdef AcCmdCLLogin
      case AcCmdCLLogin:
        {
          DummyCommand response(AcCmdCLLoginOK);
          response.data = {
            0xC2, 0x08, 0x40, 0xA7, 0xF2, 0xB7, 0xDA, 0x01,  0x94, 0xA7, 0x0C, 0x00, 0xE8, 0xE2, 0x06, 0x00,
            0x72, 0x67, 0x6E, 0x74, 0x00, 0x00, 0x01, 0x00,  0x00, 0x00, 0x0A, 0x00, 0xB1, 0x8D, 0x00, 0x00,
            0x30, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x19, 0x00, 0x00, 0x00, 0x0C, 0x01, 0x00,
            0x16, 0x57, 0x02, 0x00, 0x15, 0x41, 0x03, 0x00,  0x17, 0x44, 0x04, 0x00, 0x18, 0x53, 0x05, 0x00,
            0x12, 0x13, 0x06, 0x00, 0x82, 0x83, 0x07, 0x00,  0x20, 0x2F, 0x08, 0x00, 0x46, 0x00, 0x09, 0x00,
            0x52, 0x00, 0x0A, 0x00, 0x19, 0x00, 0x0B, 0x00,  0x0F, 0x00, 0x0C, 0x00, 0x43, 0x00, 0x2F, 0x77,
            0x69, 0x6E, 0x6B, 0x2F, 0x77, 0x61, 0x76, 0x65,  0x00, 0x54, 0x68, 0x61, 0x6E, 0x6B, 0x20, 0x79,
            0x6F, 0x75, 0x21, 0x20, 0x2F, 0x68, 0x65, 0x61,  0x72, 0x74, 0x00, 0x2F, 0x66, 0x69, 0x72, 0x65,
            0x2F, 0x66, 0x69, 0x72, 0x65, 0x2F, 0x66, 0x69,  0x72, 0x65, 0x20, 0x46, 0x69, 0x72, 0x65, 0x21,
            0x20, 0x2F, 0x66, 0x69, 0x72, 0x65, 0x2F, 0x66,  0x69, 0x72, 0x65, 0x2F, 0x66, 0x69, 0x72, 0x65,
            0x00, 0x2F, 0x73, 0x61, 0x64, 0x2F, 0x63, 0x72,  0x79, 0x20, 0x53, 0x6F, 0x72, 0x72, 0x79, 0x21,
            0x20, 0x2F, 0x63, 0x72, 0x79, 0x2F, 0x73, 0x61,  0x64, 0x00, 0x2F, 0x2D, 0x74, 0x61, 0x64, 0x61,
            0x20, 0x43, 0x6F, 0x6E, 0x67, 0x72, 0x61, 0x74,  0x75, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x73,
            0x21, 0x21, 0x21, 0x20, 0x2F, 0x74, 0x61, 0x64,  0x61, 0x00, 0x2F, 0x63, 0x6C, 0x61, 0x70, 0x20,
            0x47, 0x6F, 0x6F, 0x64, 0x20, 0x47, 0x61, 0x6D,  0x65, 0x21, 0x20, 0x2F, 0x2D, 0x63, 0x6C, 0x61,
            0x70, 0x00, 0x42, 0x65, 0x20, 0x72, 0x69, 0x67,  0x68, 0x74, 0x20, 0x62, 0x61, 0x63, 0x6B, 0x21,
            0x20, 0x50, 0x6C, 0x65, 0x61, 0x73, 0x65, 0x20,  0x77, 0x61, 0x69, 0x74, 0x20, 0x66, 0x6F, 0x72,
            0x20, 0x6D, 0x65, 0x21, 0x20, 0x2F, 0x77, 0x69,  0x6E, 0x6B, 0x00, 0x53, 0x65, 0x65, 0x20, 0x79,
            0x6F, 0x75, 0x21, 0x20, 0x2F, 0x73, 0x6D, 0x69,  0x6C, 0x65, 0x2F, 0x77, 0x61, 0x76, 0x65, 0x00,
            0x64, 0x00, 0x00, 0x00, 0x10, 0x00, 0x07, 0x18,  0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00,
            0x00, 0x00, 0x1F, 0x00, 0x01, 0x02, 0x00, 0x00,  0x00, 0x01, 0x00, 0x00, 0x00, 0x23, 0x00, 0x01,
            0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,  0x29, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x2A, 0x00, 0x01, 0x02, 0x00,  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2B, 0x00,
            0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,  0x00, 0x2E, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x6D, 0xC9, 0xD7,  0x15, 0x39, 0x89, 0x90, 0x85, 0x0C, 0x11, 0x0A,
            0x00, 0x00, 0x01, 0x01, 0x00, 0x04, 0x00, 0x08,  0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x96,
            0xA3, 0x79, 0x05, 0x21, 0x4E, 0x00, 0x00, 0x69,  0x64, 0x6F, 0x6E, 0x74, 0x75, 0x6E, 0x64, 0x65,
            0x72, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x00, 0x02,  0x03, 0x03, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x15, 0x01, 0x02, 0x02, 0x00, 0xD0, 0x07, 0x3C,
            0x00, 0x1C, 0x02, 0x00, 0x00, 0xE8, 0x03, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x03, 0x1E,
            0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE4, 0x67,
            0xA1, 0xB8, 0x02, 0x00, 0x7D, 0x2E, 0x03, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,  0x00, 0xFE, 0x01, 0x00, 0x00, 0x21, 0x04, 0x00,
            0x00, 0xF8, 0x05, 0x00, 0x00, 0xA4, 0xCF, 0x00,  0x00, 0xE4, 0x67, 0xA1, 0xB8, 0x00, 0x00, 0x00,
            0x00, 0x0A, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x04, 0x00,
            0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x02, 0x00,  0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x30, 0x75,
            0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x04, 0x00,  0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x02, 0x00,
            0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x04, 0x00,  0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x06, 0x0E, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x2B, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,  0x00, 0xDB, 0x87, 0x1B, 0xCA, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x96, 0xA3,
            0x79, 0x05, 0x12, 0x00, 0x00, 0x00, 0xE4, 0x67,  0x6E, 0x01, 0x3A, 0x00, 0x00, 0x00, 0x8E, 0x03,
            0x00, 0x00, 0xC6, 0x01, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowInventory
      case AcCmdCLShowInventory:
        {
          DummyCommand response(AcCmdCLShowInventoryOK);
          response.data = {
            0x1F, 0x4A, 0x75, 0x00, 0x02, 0x4A, 0x75, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0xB0, 0x9A, 0x00, 0x02, 0xB0, 0x9A, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x14, 0x9B, 0x00, 0x02, 0x14, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x78, 0x9B, 0x00, 0x02, 0x78, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x79, 0x9B, 0x00, 0x02, 0x79, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x7A, 0x9B, 0x00, 0x02, 0x7A, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x7B, 0x9B, 0x00, 0x02, 0x7B, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x7C, 0x9B, 0x00, 0x02, 0x7C, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x7D, 0x9B, 0x00, 0x02, 0x7D, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x7E, 0x9B, 0x00, 0x02, 0x7E, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x7F, 0x9B, 0x00, 0x02, 0x7F, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x80, 0x9B, 0x00, 0x02, 0x80, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x81, 0x9B, 0x00, 0x02, 0x81, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0xE6, 0x9B, 0x00, 0x02, 0xE6, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0xE7, 0x9B, 0x00, 0x02, 0xE7, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0xE8, 0x9B, 0x00, 0x02, 0xE8, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0xE9, 0x9B, 0x00, 0x02, 0xE9, 0x9B, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x42, 0x9C, 0x00, 0x02, 0x42, 0x9C, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x06, 0x00, 0x00,
            0x00, 0x29, 0xA0, 0x00, 0x02, 0x29, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x1C, 0x00, 0x00,
            0x00, 0x2A, 0xA0, 0x00, 0x02, 0x2A, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x0A, 0x00, 0x00,
            0x00, 0x2B, 0xA0, 0x00, 0x02, 0x2B, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x10, 0x00, 0x00,
            0x00, 0x2C, 0xA0, 0x00, 0x02, 0x2C, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x0A, 0x00, 0x00,
            0x00, 0x2E, 0xA0, 0x00, 0x02, 0x2E, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x21, 0x00, 0x00,
            0x00, 0x2F, 0xA0, 0x00, 0x02, 0x2F, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x0A, 0x00, 0x00,
            0x00, 0x30, 0xA0, 0x00, 0x02, 0x30, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x08, 0x00, 0x00,
            0x00, 0x31, 0xA0, 0x00, 0x02, 0x31, 0xA0, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x06, 0x00, 0x00,
            0x00, 0x11, 0xA4, 0x00, 0x02, 0x11, 0xA4, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x18, 0x00, 0x00,
            0x00, 0xE1, 0xAB, 0x00, 0x02, 0xE1, 0xAB, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x05, 0x00, 0x00,
            0x00, 0xE5, 0xAB, 0x00, 0x02, 0xE5, 0xAB, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x03, 0x00, 0x00,
            0x00, 0xC9, 0xAF, 0x00, 0x02, 0xC9, 0xAF, 0x00,  0x00, 0xB8, 0x1B, 0x01, 0x00, 0x02, 0x00, 0x00,
            0x00, 0x94, 0x5F, 0x01, 0x02, 0x94, 0x5F, 0x01,  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLRequestLicenseInfo
      case AcCmdCLRequestLicenseInfo:
        {
          DummyCommand response(AcCmdCLRequestLicenseInfoOK);
          response.data = {
              0x0};
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLRequestLeagueInfo
      case AcCmdCLRequestLeagueInfo:
        {
          DummyCommand response(AcCmdCLRequestLeagueInfoOK);
          response.data = {
            0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x12, 0x01, 0x01, 0x01,  0x00, 0x00, 0x34, 0x01, 0x00,
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLAchievementCompleteList
      case AcCmdCLAchievementCompleteList:
        {
          DummyCommand response(AcCmdCLAchievementCompleteListOK);
          response.data = {
            0xE8, 0xE2, 0x06, 0x00, // character id probably
                                    0x1C, 0x00, 0x28, 0x4E,  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x29, 0x4E, 0x00, 0x00, 0x00,  0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x2A, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x4E, 0x00,
            0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x2C, 0x4E, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB,  0x27, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xAC, 0x27, 0x00, 0x00,  0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
            0x00, 0xAD, 0x27, 0x00, 0x00, 0x00, 0x00, 0x01,  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAE, 0x27,
            0x00, 0x00, 0x00, 0x00, 0x01, 0xF4, 0x01, 0x00,  0x00, 0x00, 0x00, 0xAF, 0x27, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,  0xB0, 0x27, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05,
            0x00, 0x00, 0x00, 0x00, 0x00, 0xB1, 0x27, 0x00,  0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xB2, 0x27, 0x00, 0x00, 0x00, 0x00,  0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB3,
            0x27, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,  0x00, 0x00, 0x00, 0x00, 0xB4, 0x27, 0x00, 0x00,
            0x00, 0x00, 0x01, 0xF4, 0x01, 0x00, 0x00, 0x00,  0x00, 0xB5, 0x27, 0x00, 0x00, 0x00, 0x00, 0x01,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB6, 0x27,  0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
            0x00, 0xFF, 0x00, 0xB7, 0x27, 0x00, 0x00, 0x00,  0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xB8, 0x27, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03,  0x00, 0x00, 0x00, 0x00, 0x00, 0xB9, 0x27, 0x00,
            0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,  0x00, 0x00, 0xBA, 0x27, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB,  0x27, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xBC, 0x27, 0x00, 0x00,  0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00,
            0x00, 0xBD, 0x27, 0x00, 0x00, 0x00, 0x00, 0xFF,  0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xBE, 0x27,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,  0x00, 0x00, 0x00, 0xBF, 0x27, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,  0xC0, 0x27, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
            0x00, 0x00, 0x00, 0xFF, 0x00, 0xC1, 0x27, 0x00,  0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00,
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowMountList
      case AcCmdCLShowMountList:
        {
          DummyCommand response(AcCmdCLShowMountListOK);
          response.data = {
              0x0};
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowEggList
      case AcCmdCLShowEggList:
        {
          DummyCommand response(AcCmdCLShowEggListOK);
          response.data = {
              0x0};
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowCharList
      case AcCmdCLShowCharList:
        {
          DummyCommand response(AcCmdCLShowCharListOK);
          response.data = {
              0x0};
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLRequestMountEquipmentList
      case AcCmdCLRequestMountEquipmentList:
        {
          DummyCommand response(AcCmdCLRequestMountEquipmentListOK);
          response.data = {
            0xE8, 0xE2, 0x06, 0x00, // character ID?
            0x00, // length
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLEnterChannel
      case AcCmdCLEnterChannel:
        {
          DummyCommand response(AcCmdCLEnterChannelOK);
          response.data = {
              0x00, // member0
              0x00, 0x00 // member1
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLMakeRoom
      case AcCmdCLMakeRoom:
        {
          DummyCommand response(AcCmdCLMakeRoomOK);
          response.data = {
              // one of these two probably has to be the char id
              0xE8, 0xE2, 0x06, 0x00, // character id?
              0x44, 0x33, 0x22, 0x11, // probably made up
              0x7F, 0x00, 0x00, 0x01, // 127.0.0.1, game server IP
              0x2E, 0x27, // port
              0x00, // member2
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLRequestDailyQuestList
      case AcCmdCLRequestDailyQuestList:
        {  
          DummyCommand response(AcCmdCLRequestDailyQuestListOK);
          response.data = { 0xE8, 0xE2, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLRequestQuestList
      case AcCmdCLRequestQuestList:
        {
          DummyCommand response(AcCmdCLRequestQuestListOK);
          response.data = {
            0xE8, 0xE2, 0x06, 0x00, 0x0F, 0x00, 0x16, 0x2B,  0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x03, 0x17, 0x2B, 0x00, 0x00, 0x00,  0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x00, 0x03,
            0x18, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02,  0x00, 0x00, 0x00, 0x00, 0x03, 0x1B, 0x2B, 0x00,         
            0x00, 0x00, 0x00, 0x03, 0x0B, 0x00, 0x00, 0x00,  0x00, 0x03, 0x1C, 0x2B, 0x00, 0x00, 0x00, 0x00,         
            0x03, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1F,  0x2B, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00,         
            0x00, 0x00, 0x00, 0x03, 0xEA, 0x2E, 0x00, 0x00,  0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00,         
            0x03, 0xEB, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x03,  0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEC, 0x2E,         
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00,  0x00, 0x00, 0x03, 0xD2, 0x32, 0x00, 0x00, 0x00,         
            0x00, 0x01, 0x14, 0x00, 0x00, 0x00, 0x00, 0x03,  0xBA, 0x36, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02,         
            0x00, 0x00, 0x00, 0x00, 0x03, 0xBB, 0x36, 0x00,  0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00,         
            0x00, 0x03, 0xBC, 0x36, 0x00, 0x00, 0x00, 0x00,  0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x03, 0xBD,         
            0x36, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00,  0x00, 0x00, 0x00, 0x03, 0xC1, 0x36, 0x00, 0x00,         
            0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x00,  0x03,                     
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLEnterRanch
      case AcCmdCLEnterRanch:
        {
          DummyCommand response(AcCmdCLEnterRanchOK);
          response.data = { 
            0xE8, 0xE2, 0x06, 0x00, // character id?
            0x44, 0x33, 0x22, 0x11, // probably made up
            127, 0, 0, 1,
            0x2e, 0x27,
          };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLGetMessengerInfo
      case AcCmdCLGetMessengerInfo:
        {
          DummyCommand response(AcCmdCLGetMessengerInfoOK);
          response.data = { 0x03, 0xBB, 0x2D, 0xD6, 0x88, 0xF3, 0x51, 0xEE,  0x68, 0x42 };
          send_command(_socket, response);
        }
        break;
      #endif

      #ifdef AcCmdCLHeartbeat
      case AcCmdCLHeartbeat:
        // Do nothing
        break;
      #endif

      #ifdef AcCmdCRHeartbeat
      case AcCmdCRHeartbeat:
        // Do nothing
        break;
      #endif

      default:
        std::cout << "WARNING! Packet " << GetMessageName(message_magic.id) << "(0x" << std::hex << message_magic.id << std::dec << ") not handled" << std::endl << std::endl;
        break;
    }
    
    read_loop();
  });
}

void alicia::Server::host()
{
  asio::ip::tcp::endpoint server_endpoint(asio::ip::tcp::v4(), 10030);
  printf("Hosting the server on port 10030\n");
  _acceptor.open(server_endpoint.protocol());
  _acceptor.bind(server_endpoint);
  _acceptor.listen();
  accept_loop();
}

void alicia::Server::accept_loop()
{
  _acceptor.async_accept([&](boost::system::error_code error, asio::ip::tcp::socket client_socket) {
    printf("+++ CONN %s:%d\n\n", client_socket.remote_endpoint().address().to_string().c_str(), client_socket.remote_endpoint().port());
    const auto [itr, _] = _clients.emplace(client_id++, std::move(client_socket));
    itr->second.read_loop();

    accept_loop();
  });
}