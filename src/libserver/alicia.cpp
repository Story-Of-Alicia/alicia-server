#include <bit>
#include <iostream>

#include "libserver/alicia.hpp"
#include "libserver/mapping.hpp"

uint16_t MUTE_COMMAND_IDS[] = {
  0x3cbb, // Messenger
  0x3c87, // Messenger

  #ifdef AcCmdCLHeartbeat
  AcCmdCLHeartbeat,
  #endif

  #ifdef AcCmdCRHeartbeat
  AcCmdCRHeartbeat,
  #endif

  #ifdef AcCmdCRRanchSnapshot
  AcCmdCRRanchSnapshot,
  #endif

  #ifdef AcCmdCRRanchSnapshotNotify
  AcCmdCRRanchSnapshotNotify
  #endif
};

namespace
{

} // namespace anon

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

void alicia::Client::read_loop(Server& server)
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
    if(data.size() > 0)
    {
      xor_codec_cpp(this->xor_key, data);
      this->rotate_xor_key();
    }

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
          FILETIME time{};
          ZeroMemory(&time, sizeof(time));
          GetSystemTimeAsFileTime(&time);

          DummyCommand response(AcCmdCLLoginOK);
          response.data = {
            // Lobby filetime lsb
            time.dwLowDateTime >> 0 & 0xFF,
            time.dwLowDateTime >> 8 & 0xFF,
            time.dwLowDateTime >> 16 & 0xFF,
            time.dwLowDateTime >> 24 & 0xFF,
            // Lobby filetime msb
            time.dwHighDateTime >> 0 & 0xFF,
            time.dwHighDateTime >> 8 & 0xFF,
            time.dwHighDateTime >> 16 & 0xFF,
            time.dwHighDateTime >> 24 & 0xFF,

            0x94, 0xA7, 0x0C, 0x00,

            0xE8, 0xE2, 0x06, 0x00, // Self UID
            'r', 'g', 'n', 't', 0x00, // Nick name ("rgnt\0")
            'W', 'e', 'l', 'c', 'o', 'm', 'e', ' ', 't', 'o', ' ', 'S', 't', 'o', 'r', 'y', ' ', 'o', 'f', ' ', 'A', 'l', 'i', 'c', 'i', 'a', '!', 0x00, // motd, 100 chars long
            0x01, // profile gender:
                  // 0x00 - baby
                  // 0x01 - boy
                  // 0x02 - girl
            'T', 'h', 'i', 's', ' ', 'p', 'e', 'r', 's', 'o', 'n', ' ', 'i', 's', ' ', 'm', 'e', 'n', 't', 'a', 'l', 'l', 'y', ' ', 'u', 'n', 's', 't', 'a', 'b', 'l', 'e', 0x00, // info, 100 chars long

            0x01, // Character items (equipment), max 16 elements
                0x01, 0x00, 0x00, 0x00, // Item UID
                0x38, 0x75, 0x00, 0x00, // Item TID
                0x00, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, // Count
                  // 4 byte - type
                  // 4 byte - TID
                  // 4 byte - ???
                  // 4 byte - ???

            0x01, // Horse items (equipment), max 250 elements
              0x28, 0x4E, 0x00, 0x02, // Item UID
              0x28, 0x4E, 0x00, 0x00, // Item TID - forest armor
              0x00, 0x00, 0x00, 0x00,
              0x01, 0x00, 0x00, 0x00, // Count

            0xA1, 0x00, // Level
            0xFF, 0x00, 0x00, 0x00, // Carrots
            0x30, 0x61, 0x00, 0x00, // Unk7
            0xFF, 0x00, 0x00, 0x00, // Unk8
            0xFF, // Unk9

            // Unk10: Data structure with keybinds and macros
            0x19, 0x00, 0x00, 0x00, // Unk10.Unk0: Bitmask indicating what this structure contains
            // & 1 != 0: Keyboard bindings
            0x0C, // List size, max 16 elements, keybinds?
              0x01, 0x00, // Index
              0x16,
              0x57, // W

              0x02, 0x00,
              0x15,
              0x41, // A

              0x03, 0x00,
              0x17,
              0x44, // D

              0x04, 0x00,
              0x18,
              0x53, // S

              0x05, 0x00,
              0x12,
              0x13,

              0x06, 0x00,
              0x82,
              0x83,

              0x07, 0x00,
              0x20,
              0x2F,

              0x08, 0x00,
              0x46,
              0x00,

              0x09, 0x00,
              0x52,
              0x00,

              0x0A, 0x00,
              0x19,
              0x00,

              0x0B, 0x00,
              0x0F,
              0x00,

              0x0C, 0x00,
              0x43,
              0x00,

            // & 8 != 0: Macros, array of 8 strings
            /* "/wink/wave" */ 0x2F, 0x77, 0x69, 0x6E, 0x6B, 0x2F, 0x77, 0x61, 0x76, 0x65, 0x00,
            /* "Thank you! /heart */ 0x54, 0x68, 0x61, 0x6E, 0x6B, 0x20, 0x79, 0x6F, 0x75, 0x21, 0x20, 0x2F, 0x68, 0x65, 0x61, 0x72, 0x74, 0x00,
            /* "/fire/fire/fire Fire! /fire/fire/fire" */ 0x2F, 0x66, 0x69, 0x72, 0x65, 0x2F, 0x66, 0x69, 0x72, 0x65, 0x2F, 0x66, 0x69, 0x72, 0x65, 0x20, 0x46, 0x69, 0x72, 0x65, 0x21, 0x20, 0x2F, 0x66, 0x69, 0x72, 0x65, 0x2F, 0x66,  0x69, 0x72, 0x65, 0x2F, 0x66, 0x69, 0x72, 0x65, 0x00,
            /* "/sad/cry Sorry! /cry/sad" */ 0x2F, 0x73, 0x61, 0x64, 0x2F, 0x63, 0x72,  0x79, 0x20, 0x53, 0x6F, 0x72, 0x72, 0x79, 0x21, 0x20, 0x2F, 0x63, 0x72, 0x79, 0x2F, 0x73, 0x61,  0x64, 0x00,
            /* "/-tada Congralutations!!! /tada" */ 0x2F, 0x2D, 0x74, 0x61, 0x64, 0x61, 0x20, 0x43, 0x6F, 0x6E, 0x67, 0x72, 0x61, 0x74,  0x75, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x73, 0x21, 0x21, 0x21, 0x20, 0x2F, 0x74, 0x61, 0x64,  0x61, 0x00,
            /* "/clap Good Gam1 /-clap" */ 0x2F, 0x63, 0x6C, 0x61, 0x70, 0x20, 0x47, 0x6F, 0x6F, 0x64, 0x20, 0x47, 0x61, 0x6D,  0x65, 0x21, 0x20, 0x2F, 0x2D, 0x63, 0x6C, 0x61, 0x70, 0x00,
            /* "Be right back! Please wait for me! /wink" */ 0x42, 0x65, 0x20, 0x72, 0x69, 0x67,  0x68, 0x74, 0x20, 0x62, 0x61, 0x63, 0x6B, 0x21, 0x20, 0x50, 0x6C, 0x65, 0x61, 0x73, 0x65, 0x20,  0x77, 0x61, 0x69, 0x74, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x6D, 0x65, 0x21, 0x20, 0x2F, 0x77, 0x69,  0x6E, 0x6B, 0x00,
            /* "See you! /smile/wave" */ 0x53, 0x65, 0x65, 0x20, 0x79, 0x6F, 0x75, 0x21, 0x20, 0x2F, 0x73, 0x6D, 0x69,  0x6C, 0x65, 0x2F, 0x77, 0x61, 0x76, 0x65, 0x00,

            // & 16 != 0: Single option
            0x64, 0x00, 0x00, 0x00,

            // & 32 != 0: Gamepad bindings (Not present in this packet)

            // These two affect the age
            0x13, // 0x0C - <12
                  // 0x0D - 13-15
                  // 0x10 - 16-18
                  // 0x13 - 19+
            0x00, // Unk10.UnkAnotherSomething, 1 byte

            // Unk11: Another structure
            0x07, // List size, max 16 values
              0x18,  0x00,
              0x01, // Sublist size, seems like its deserializing in a very dangerous way (arr[index] = value), i may be wrong though
                0x02, 0x00, 0x00, 0x00, // Index
                0x01, 0x00, 0x00, 0x00, // Value

              0x1F, 0x00,
              0x01,
                0x02, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,

              0x23, 0x00,
              0x01,
                0x02, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,

              0x29, 0x00,
              0x01,
                0x02, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,

              0x2A, 0x00,
              0x01,
                0x02, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,

              0x2B, 0x00,
              0x01,
                0x02, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,

              0x2E, 0x00,
              0x01,
                0x02, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,

            0x00, // Unk12: Another string

            127, 0, 0, 1, // Address
            0x2E, 0x27, // port

            0x00, 0x00, 0x00, 0x00, // Lobby scrambling constant

            // Unk15: Another small structure
            // CharDefaultPartInfo (_ClientCharDefaultPartInfo)
            0x0A, // .Id -0x0A (10) is male and 0x14 (female)
            0x01, // .MouthPartSerial
            0x02, // .FacePartSerial
            0x01, // ??

            0xFF, 0xFF,
            0x04, 0x00, // Character.HeadSize
            0x08, 0x00, // Character.Height
            0x08, 0x00, // Character.ThighVolume
            0x08, 0x00, // Character.LegVolume
            0xFF, 0x00, //

            // Horse: Big ass structure now, probably horse info
            // Horse.TIDs
            0x96, 0xA3, 0x79, 0x05, // Horse.TIDs.HorseUID Unique horse identifier
            0x21, 0x4E, 0x00, 0x00, // Horse.TIDs.MountTID Horse model identifier

            /* Horse name: "idontunderstand" */ 0x69, 0x64, 0x6F, 0x6E, 0x74, 0x75, 0x6E, 0x64, 0x65, 0x72, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x00,
            // Horse.Appearance: Structure. Probably horse appearance

            // MountPartSet
            0x01, // .SkinId (MountSkinInfo)
            0x04, // .ManeId (MountManeInfo)
            0x04, // .TailId (MountTailInfo)
            0x05, // .FaceId (MountFaceInfo)
            0x00, // .Fig_Scale
            0x00, // .Fig_LegLength
            0x00, // .Fig_LegVol
            0x00, // .Fig_BodyLength
            0x00, // .Fig_BodyVol

            // MountMultiAbility
            0x09, 0x00, 0x00, 0x00, // .Agility
            0x09, 0x00, 0x00, 0x00, // spirit
            0x09, 0x00, 0x00, 0x00, // speed
            0x09, 0x00, 0x00, 0x00, // strength
            0x09, 0x00, 0x00, 0x00, // .Ambition

            0x00, 0x00, 0x00, 0x00, // Horse.Rating
            0x15, // Horse.Class
            0x01, // Horse.Unk4
            0x05, // Horse.Grade
            0x00, 0x00, // Horse.AvailableGrowthPoints

            // Horse.Unk7: An array of size 7. Each element has two 2 byte values
            0xFF, 0xFF, // Stamina
            0xFF, 0xFF, // Attractiveness
            0xFF, 0xFF, // Hunger
            0x00, 0x00, //

            0xE8, 0x03,
            0x00, 0x00,

            0x00, 0x00,
            0x00, 0x00,

            0xE8, 0x03,
            0x1E, 0x00,

            0x0A, 0x00,
            0x0A, 0x00,

            0x0A, 0x00,
            0x00, 0x00,

            // More horse fields
            0x00,
            0x00, 0x00, 0x00, 0x00, 
            0xE4, 0x67, 0xA1, 0xB8,
            0x02,
            0x00, 
            0xFF, 0x00, 0x00, 0x00, // int32 - Class progression
            0x00, 0x00, 0x00, 0x00,
            0x00,
            0x00,
            0xFF,
            0x00, 
            0x04, 
            0x00,
            0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x01, 0x00, 

            // Horse field: Horse Mastery Array of four 4 int values
            0xFE, 0x01, 0x00, 0x00, // speed booster/magic use
            0x21, 0x04, 0x00, 0x00, // jumps
            0xF8, 0x05, 0x00, 0x00, // sliding
            0xA4, 0xCF, 0x00, 0x00, // gliding, value here is divided by 10 by client
            
            0xE4, 0x67, 0xA1, 0xB8, 
            0x00, 0x00, 0x00, 0x00, 
            
            // Unk16: List
            0x0A, 
              0x06, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 
              
              0x0F, 0x00, 0x00, 0x00, 
              0x04, 0x00, 0x00, 0x00, 
              
              0x1B, 0x00, 0x00, 0x00, 
              0x02, 0x00, 0x00, 0x00,
              
              0x1E, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              
              0x1F, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              
              0x25, 0x00, 0x00, 0x00,
              0x30, 0x75, 0x00, 0x00,
              
              0x35, 0x00, 0x00, 0x00,
              0x04, 0x00, 0x00, 0x00,
              
              0x42, 0x00, 0x00, 0x00,
              0x02, 0x00, 0x00, 0x00,
              
              0x43, 0x00, 0x00, 0x00,
              0x04, 0x00, 0x00, 0x00,
              
              0x45, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,

            // Unk17: Bitset that im not too sure how it gets interpreted
            0x06, 0x0E, 0x00, 0x00, 
            
            // Unk18: List of 3 shorts?
            0x00, 0x00, 
            0x00, 0x00, 
            0x00, 0x00, 
            
            0x00, 0x00, 0x00, 0x00, // Unk19

            // Unk20
            0x04,
            0x2B, 0x00, 0x00, 0x00,
            0x04, 0x00,
            
            // Unk21: List of byte byte
            0x00, // List size
            
            // Unk22: List of short byte byte
            0x00, // List size

            0xDB, 0x87, 0x1B, 0xCA,  // Unk23
            
            // Unk24: Buffer::ReadPlayerRelatedThing, shared with the structures in EnterRanch
            0x00, 0x00, 0x00, 0x00, 
            0x01, 
            0x00, 0x00, 0x00, 0x00, 
            0x00, // string
            0x00, 
            0x00, 0x00, 0x00, 0x00,            
            0x00, // Goes ignored?

            0x04, // Unk25
            
            // Unk26: Buffer::ReadAnotherPlayerRelatedSomething, also shared
            0x96, 0xA3, 0x79, 0x05, // Horse UID?
            0x12, 0x00, 0x00, 0x00,             
            0xE4, 0x67, 0x6E, 0x01, 

            0x3A, 0x00, 0x00, 0x00, // Unk27
            0x8E, 0x03, 0x00, 0x00, // Unk28
            0xC6, 0x01, 0x00, 0x00, // Unk29

            // Buffer::ReadYetAnotherPlayerRelatedSomething, also shared
            0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 
            0x00, // string
            0x00, 0x00, 0x00, 0x00
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowInventory
      case AcCmdCLShowInventory:
        {
          DummyCommand response(AcCmdCLShowInventoryOK);
          response.data = {
            32, // count of equipment items, max 251

              // Equipment Item
              0x4A, 0x75, 0x00, 0x02, //
              0x4A, 0x75, 0x00, 0x00, //
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0xB0, 0x9A, 0x00, 0x02,
              0xB0, 0x9A, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x14, 0x9B, 0x00, 0x02,
              0x14, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x78, 0x9B, 0x00, 0x02,
              0x78, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x79, 0x9B, 0x00, 0x02,
              0x79, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x7A, 0x9B, 0x00, 0x02,
              0x7A, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x7B, 0x9B, 0x00, 0x02,
              0x7B, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x7C, 0x9B, 0x00, 0x02,
              0x7C, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x7D, 0x9B, 0x00, 0x02,
              0x7D, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x7E, 0x9B, 0x00, 0x02,
              0x7E, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x7F, 0x9B, 0x00, 0x02,
              0x7F, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x80, 0x9B, 0x00, 0x02,
              0x80, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x81, 0x9B, 0x00, 0x02,
              0x81, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0xE6, 0x9B, 0x00, 0x02,
              0xE6, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0xE7, 0x9B, 0x00, 0x02,
              0xE7, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0xE8, 0x9B, 0x00, 0x02,
              0xE8, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0xE9, 0x9B, 0x00, 0x02,
              0xE9, 0x9B, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x01, 0x00, 0x00, 0x00,

              0x42, 0x9C, 0x00, 0x02,
              0x42, 0x9C, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x06, 0x00, 0x00, 0x00,

              0x29, 0xA0, 0x00, 0x02,
              0x29, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x1C, 0x00, 0x00, 0x00,

              0x2A, 0xA0, 0x00, 0x02,
              0x2A, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x0A, 0x00, 0x00, 0x00,

              0x2B, 0xA0, 0x00, 0x02,
              0x2B, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x10, 0x00, 0x00, 0x00,

              0x2C, 0xA0, 0x00, 0x02,
              0x2C, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x0A, 0x00, 0x00, 0x00,

              0x2E, 0xA0, 0x00, 0x02,
              0x2E, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x21, 0x00, 0x00, 0x00,

              0x2F, 0xA0, 0x00, 0x02,
              0x2F, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x0A, 0x00, 0x00, 0x00,

              0x30, 0xA0, 0x00, 0x02,
              0x30, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x08, 0x00, 0x00, 0x00,

              0x31, 0xA0, 0x00, 0x02,
              0x31, 0xA0, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x06, 0x00, 0x00, 0x00,

              0x11, 0xA4, 0x00, 0x02,
              0x11, 0xA4, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x18, 0x00, 0x00, 0x00,

              0x12, 0xA4, 0x00, 0x02,
              0x12, 0xA4, 0x00, 0x00, // training carrot on a stick
              0xB8, 0x1B, 0x01, 0x00,
              0x18, 0x00, 0x00, 0x00,

              0xE1, 0xAB, 0x00, 0x02,
              0xE1, 0xAB, 0x00, 0x00, // training bow
              0xB8, 0x1B, 0x01, 0x00,
              0x05, 0x00, 0x00, 0x00,

              0xE5, 0xAB, 0x00, 0x02,
              0xE5, 0xAB, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x03, 0x00, 0x00, 0x00,

              0xC9, 0xAF, 0x00, 0x02,
              0xC9, 0xAF, 0x00, 0x00,
              0xB8, 0x1B, 0x01, 0x00,
              0x02, 0x00, 0x00, 0x00,

              0x94, 0x5F, 0x01, 0x02,
              0x94, 0x5F, 0x01, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x01, 0x00, 0x00, 0x00,

            0x00
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCRUseItem
          case AcCmdCRUseItem: {
            DummyCommand response(AcCmdCRUseItemOK);
            response.data = {
              request.data[0], request.data[1], request.data[2], request.data[3], // item TID
              0x01, 0x00,
              0x00, 0x00, 0x00, 0x00, // enum
              0x00,
            };
            this->send_command(response);
          }
      #endif


      #ifdef AcCmdCLRequestLicenseInfo
      case AcCmdCLRequestLicenseInfo:
        {
          DummyCommand response(AcCmdCLRequestLicenseInfoOK);
          response.data = {
              0x0};
          this->send_command(response);
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
          this->send_command(response);
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
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowMountList
      case AcCmdCLShowMountList:
        {
          DummyCommand response(AcCmdCLShowMountListOK);
          response.data = {
              0x0};
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCRMountFamilyTree
      case AcCmdCRMountFamilyTree:
        {
          DummyCommand response(AcCmdCRMountFamilyTreeOK);
          response.data = {
            // int
            0x00, 0x00, 0x00, 0x00,

            // number of items up to 6
            0x06,

            // item structure
            // byte
            // string max length 17
            // byte
            // 2 bytes

            0x00, 
            't', 'e', 's', 't', '1', 0x00, 
            0x02, 
            0x02, 0x00, 

            0x01, 
            't', 'e', 's', 't', '2', 0x00, 
            0x02, 
            0x02, 0x00, 

            0x02, 
            't', 'e', 's', 't', '3', 0x00, 
            0x02,
            0x02, 0x00,

            0x03,
            't', 'e', 's', 't', '4', 0x00,
            0x02,
            0x02, 0x00,

            0x04,
            't', 'e', 's', 't', '5', 0x00,
            0x02,
            0x02, 0x00,

            0x05,
            't', 'e', 's', 't', '6', 0x00,
            0x02,
            0x02, 0x00,
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowEggList
      case AcCmdCLShowEggList:
        {
          DummyCommand response(AcCmdCLShowEggListOK);
          response.data = {
              0x0};
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCLShowCharList
      case AcCmdCLShowCharList:
        {
          DummyCommand response(AcCmdCLShowCharListOK);
          response.data = {
              0x0};
          this->send_command(response);
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
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCLRequestPersonalInfo
      case AcCmdCLRequestPersonalInfo:
      {
        // Read uVar2 from the request data
        if (request.data.size() < 8) {
          // Handle error: not enough data in the request
          std::cerr << "Error: Not enough data in the request!" << std::endl;
          return;
        }
        // first 4 bytes of the request is player UID (request.data[0 - 3])
        uint8_t uVar2 = request.data[4]; // The fifth byte (index 4) contains uVar2

        DummyCommand response(AcCmdLCPersonalInfo);
        response.data = {
          0x01, 0x00, 0x00, 0x00,   // unknown, used in all requests
          uVar2, 0x00, 0x00, 0x00,   // uVar2 value (6, 7, or 8)
        };

        if (uVar2 == 6) {
          // append data for when client is requesting 6
          std::vector<uint8_t> response1_data = {
              0x08, 0x07, 0x06, 0x05,   // 4 bytes
              0x0C, 0x0B, 0x0A, 0x09,   // 4 bytes
              0x10, 0x0F, 0x0E, 0x0D,   // 4 bytes
              0x00, 0x00, 0x20, 0x41,   // float 4 bytes, 10.0f
              0x00, 0x00, 0x40, 0x41,   // float 4 bytes, 12.0f
              0x00, 0x01,               // 2 bytes
              0x02, 0x03,               // 2 bytes
              0x04, 0x05,               // 2 bytes
              0x06, 0x07,               // 2 bytes
              0x00, 0x00, 0x80, 0x3F,   // float 4 bytes, 1.0f
              0x00, 0x00, 0xA0, 0x40,   // float 4 bytes, 5.0f
              0x00, 0x00, 0xC0, 0x40,   // float 4 bytes, 6.0f)
              0x18, 0x17, 0x16, 0x15,   // 4 bytes
              0x08, 0x09,               // 2 bytes
              0x0A, 0x0B,               // 2 bytes
              0x0C, 0x0D,               // 2 bytes
            
              0x74, 0x65, 0x73, 0x74, 0x31, 0x00, // string "test1" null-terminated
            
              0x20, 0x1F, 0x1E, 0x1D,   // 4 bytes
              0x24, 0x23, 0x22, 0x21,   // 4 bytes
            
              // String data (null-terminated)
              0x74, 0x65, 0x73, 0x74, 0x32, 0x00, // string "test2" null-terminated
            
              0x0E, 0x0F,               // 2 bytes
              0x10, 0x11,               // 2 bytes
              0x12, 0x13,               // 2 bytes
              0x00, 0x00, 0xE0, 0x40,   // float 4 bytes, 7.0f)
              0x00, 0x00, 0x00, 0x41,   // float 4 bytes, 8.0f)
              0x00, 0x00, 0x10, 0x41,   // float 4 bytes, 9.0f)
            
              // String data (null-terminated)
              0x74, 0x65, 0x73, 0x74, 0x33, 0x00, // String data "test3"
            
              0x01,                     // 1 byte
              0x00                      // 1 byte
          };
          response.data.insert(response.data.end(), response1_data.begin(), response1_data.end());
          }
        else if (uVar2 == 7) 
        {
          // append data for when client is requesting 7
          std::vector<uint8_t> response2_data = {
            // FUN_004c1130
            0x00, 0x00, 0x00, 0x00,   // 4 bytes
            0x00, 0x00, 0x00, 0x00,   // 4 bytes
            0x00, 0x00, 0x00, 0x00,   // 4 bytes

            // FUN_004c0e70
            0x02,   // Number of structs, max of 128

            // FUN_004c05b0 (2 structs, 24 bytes each)
            // Structure 1
            0x00, 0x00,               // 2 bytes
            0x00, 0x00, 0x00, 0x00,   // 4 bytes
            0x00, 0x00, 0x00, 0x00,   // 4 bytes
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 12 byte memcpy

            // Structure 2
            0x00, 0x00,               // 2 bytes
            0x00, 0x00, 0x00, 0x00,   // 4 bytes
            0x00, 0x00, 0x00, 0x00,   // 4 bytes
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 12 byte memcpy
          };
          response.data.insert(response.data.end(), response2_data.begin(), response2_data.end());
        }
        else if (uVar2 == 8) 
        {
          // append data for when client is requesting 8
          std::vector<uint8_t> response3_data = {
              0x02,  // 2 structs, max size 31

              // FUN_004c07e0 structures
              // struct 1
              0x00, 0x00, 0x00, 0x00,  // First 4-byte value
              0x00, 0x00, 0x00, 0x00,  // Second 4-byte value

              // struct 2
              0x00, 0x00, 0x00, 0x00,  // First 4-byte value
              0x00, 0x00, 0x00, 0x00   // Second 4-byte value
          };
          response.data.insert(response.data.end(), response3_data.begin(), response3_data.end());
        }
        this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCLEnterChannel
      case AcCmdCLEnterChannel:
        {
          // Requests send a single byte
          uint8_t unk0 = request.data[0];

          DummyCommand response(AcCmdCLEnterChannelOK);
          response.data = {
              unk0,
              0x01, 0x00
          };
          this->send_command(response);
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
              0x2F, 0x27, // port
              0x00, // member2
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCLRequestDailyQuestList
      case AcCmdCLRequestDailyQuestList:
        {  
          DummyCommand response(AcCmdCLRequestDailyQuestListOK);
          response.data = { 0xE8, 0xE2, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 };
          this->send_command(response);
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
          this->send_command(response);
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
            0x30, 0x27,
          };
          this->send_command(response);
        }
        break;
      #endif
    
      #ifdef AcCmdCREnterRanch
      case AcCmdCREnterRanch: {
          // Received request contents:
          // E8 E2 06 00 44 33 22 11  E8 E2 06 00 21 B2 64

          DummyCommand response(AcCmdCREnterRanchOK);
          response.data = {
              0x01, 0x00, 0x00, 0x00, // ranch id?
              't', 'e', 's', 't', '1', 0x00,// string max length 17
              'R', 'a', 'n', 'c', 'h', 0x00,// Ranch name, max length 61

              // Structure with a list of byte and horse
              // Likely index and horses in the ranch
              0x01, // List size, max 10

                0x01, 0x00, // Ranch index
                // Horse: Big ass structure now, probably horse info
                // Horse.TIDs
                0x99, 0xA3, 0x79, 0x05, // Horse.TIDs.MountTID Unique horse identifier
                0x21, 0x4E, 0x00, 0x00, // Horse.TIDs.HorseTID Horse model
                /* Horse name: */ 'R', 'a', 'm', 'o', 'n', 0,
                // Horse.Appearance: Structure. Probably horse appearance
                0x02,
                0x03,
                0x03,
                0x03,
                0x04,
                0x04,
                0x05,
                0x03,
                0x04,
                // Horse.Stats
                0x09, 0x00, 0x00, 0x00, // agility
                0x09, 0x00, 0x00, 0x00, // spirit
                0x09, 0x00, 0x00, 0x00, // speed
                0x09, 0x00, 0x00, 0x00, // strength
                0x13, 0x00, 0x00, 0x00, // control

                0x00, 0x00, 0x00, 0x00, // Horse.Rating
                0x15, // Horse.Class
                0x01, // Horse.Unk4
                0x05, // Horse.Unk5
                0x02, 0x00, // Horse.AvailableGrowthPoints

                // Horse.Unk7: An array of size 7. Each element has two 2 byte values
                0xD0, 0x07,
                0x3C, 0x00,

                0x1C, 0x02,
                0x00, 0x00,

                0xE8, 0x03,
                0x00, 0x00,

                0x00, 0x00,
                0x00, 0x00,

                0xE8, 0x03,
                0x1E, 0x00,

                0x0A, 0x00,
                0x0A, 0x00,

                0x0A, 0x00,
                0x00, 0x00,

                // More horse fields
                0x00,
                0x00, 0x00, 0x00, 0x00, 
                0xE4, 0x67, 0xA1, 0xB8, 
                0x02, 
                0x00, 
                0x7D, 0x2E, 0x03, 0x00,
                0x00, 0x00, 0x00, 0x00, 
                0x00, 
                0x00, 
                0x00, 
                0x00, 
                0x04, 
                0x00,
                0x00,
                0x00, 0x00,
                0x00, 0x00,
                0x01, 0x00, 

                // Horse field: Horse Mastery Array of four 4 int values
                0xFE, 0x01, 0x00, 0x00, // speed booster/magic use
                0x21, 0x04, 0x00, 0x00, // jumps
                0xF8, 0x05, 0x00, 0x00, // sliding
                0xA4, 0xCF, 0x00, 0x00, // gliding, value here is divided by 10 by client
                
                0xE4, 0x67, 0xA1, 0xB8, 
                0x00, 0x00, 0x00, 0x00,


              // Structure consisting of:
              // uint
              // string
              // byte
              // byte
              // byte
              // string
              // AcCmdCLLoginOK.Unk15
              // Horse
              // list of equipment (AcCmdCLLoginOK_Unk3_Element)
              // structure with:
              //  uint
              //  byte
              //  uint
              //  string
              //  byte
              //  uint
              // short
              // byte
              // byte
              // structure with:
              //  uint
              //  uint
              // structure with:
              //  uint
              //  uint
              //  string
              //  uint
              // structure with:
              //  byte
              //  byte
              // Very likely other players in the ranch
              0x02, // List size, max 201

                // YOURSELF (Required or else you dont spawn in the ranch)
                0xE8, 0xE2, 0x06, 0x00, // Self UID
                'r', 'g', 'n', 't', 0x00, // Nick name ("rgnt\0")
                0x01, // profile gender
                1,
                1,
                'T', 'h', 'i', 's', ' ', 'p', 'e', 'r', 's', 'o', 'n', ' ', 'i', 's', ' ', 'm', 'e', 'n', 't', 'a', 'l', 'l', 'y', ' ', 'u', 'n', 's', 't', 'a', 'b', 'l', 'e', 0x00, // info, 100 chars long
                
                // Unk15: Another small structure
                0x0A,
                0x00,
                0x00,
                0x01,
                0x01, 0x00,
                0x04, 0x00,
                0x08, 0x00,
                0x08, 0x00,
                0x08, 0x00,
                0x00, 0x00,   

                // Horse: Big ass structure now, probably horse info
                // Horse.TIDs
                0x96, 0xA3, 0x79, 0x06, // Horse.TIDs.MountTID Unique horse identifier
                0x21, 0x4E, 0x00, 0x00, // Horse.TIDs.HorseTID Horse model
                /* Horse name: "idontunderstand" */ 0x69, 0x64, 0x6F, 0x6E, 0x74, 0x75, 0x6E, 0x64, 0x65, 0x72, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x00,
                // Horse.Appearance: Structure. Probably horse appearance
                0x02,
                0x03,
                0x03,
                0x03,
                0x04,
                0x04,
                0x05,
                0x03,
                0x04,
                // Horse.Stats
                0x04, 0x00, 0x00, 0x00, // agility
                0x03, 0x00, 0x00, 0x00, // spirit
                0x02, 0x00, 0x00, 0x00, // speed
                0x01, 0x00, 0x00, 0x00, // strength 
                0x13, 0x00, 0x00, 0x00, // control

                0x00, 0x00, 0x00, 0x00, // Horse.Rating
                0x15, // Horse.Class
                0x01, // Horse.Unk4
                0x02, // Horse.Unk5
                0x02, 0x00, // Horse.AvailableGrowthPoints

                // Horse.Unk7: An array of size 7. Each element has two 2 byte values
                0xD0, 0x07,
                0x3C, 0x00,

                0x1C, 0x02,
                0x00, 0x00,

                0xE8, 0x03,
                0x00, 0x00,

                0x00, 0x00,
                0x00, 0x00,

                0xE8, 0x03,
                0x1E, 0x00,

                0x0A, 0x00,
                0x0A, 0x00,

                0x0A, 0x00,
                0x00, 0x00,

                // More horse fields
                0x00,
                0x00, 0x00, 0x00, 0x00, 
                0xE4, 0x67, 0xA1, 0xB8, 
                0x02, 
                0x00, 
                0x7D, 0x2E, 0x03, 0x00,
                0x00, 0x00, 0x00, 0x00, 
                0x00, 
                0x00, 
                0x00, 
                0x00, 
                0x04, 
                0x00,
                0x00,
                0x00, 0x00,
                0x00, 0x00,
                0x01, 0x00,

                // Horse field: Horse Mastery Array of four 4 int values
                0xFE, 0x01, 0x00, 0x00, // speed booster/magic use
                0x21, 0x04, 0x00, 0x00, // jumps
                0xF8, 0x05, 0x00, 0x00, // sliding
                0xA4, 0xCF, 0x00, 0x00, // gliding, value here is divided by 10 by client
                
                0xE4, 0x67, 0xA1, 0xB8, 
                0x00, 0x00, 0x00, 0x00, 

                // Back to player fields
                0x01, // Equipment list size: List size, max 16 elements
                0x01, 0x00, 0x00, 0x00, // 4 byte - type
                0x31, 0x75, 0x00, 0x00, // 4 byte - TID
                0x01, 0x00, 0x00, 0x00, // 4 byte - ???
                0x01, 0x00, 0x00, 0x00, // 4 byte - ???

                // Buffer::ReadPlayerRelatedThing, shared with the structures in LoginOK
                0x00, 0x00, 0x00, 0x00, 
                0x01, 
                0x00, 0x00, 0x00, 0x00, 
                0x00, // string
                0x00, 
                0x00, 0x00, 0x00, 0x00,            
                0x00, // Goes ignored?
                  
                0x02, 0x00, // RANCH INDEX
                0x00,
                0x00,

                // Buffer::ReadAnotherPlayerRelatedSomething, also shared
                0x96, 0xA3, 0x79, 0x06, // Horse UID?
                0x12, 0x00, 0x00, 0x00,             
                0xE4, 0x67, 0x6E, 0x01,

                // Buffer::ReadYetAnotherPlayerRelatedSomething, also shared
                0x00, 0x00, 0x00, 0x00, 
                0x00, 0x00, 0x00, 0x00, 
                0x00, // string
                0x00, 0x00, 0x00, 0x00,
                
                0x00, 
                0x00, 


                // ANOTHER PLAYER
                0x02, 0x00, 0x00, 0x00, // Self UID
                'L', 'a', 'i', 't', 'h', 0x00, // Nick name
                0x01, // profile gender
                1,
                1,
                'H', 'o', 'l', 'a', 0x00, // info, 100 chars long
                
                // Unk15: Another small structure
                0x0A,
                0x00,
                0x00,
                0x01,
                0x01, 0x00,
                0x04, 0x00,
                0x08, 0x00,
                0x08, 0x00,
                0x08, 0x00,
                0x00, 0x00,
                
                // Horse: Big ass structure now, probably horse info
                // Horse.TIDs
                0x97, 0xA3, 0x79, 0x05, // Horse.TIDs.MountTID Unique horse identifier
                0x21, 0x4E, 0x00, 0x00, // Horse.TIDs.HorseTID Horse model
                /* Horse name: "idontunderstand" */ 'R', 'o', 'c', 'i', 'n', 'a', 'n', 't', 'e', 0x00,
                // Horse.Appearance: Structure. Probably horse appearance
                0x02,
                0x03,
                0x03,
                0x03,
                0x04,
                0x04,
                0x05,
                0x03,
                0x04,
                // Horse.Stats
                0x04, 0x00, 0x00, 0x00, // agility
                0x03, 0x00, 0x00, 0x00, // spirit
                0x02, 0x00, 0x00, 0x00, // speed
                0x01, 0x00, 0x00, 0x00, // strength
                0x13, 0x00, 0x00, 0x00, // control
                
                0x00, 0x00, 0x00, 0x00, // Horse.Rating
                0x15, // Horse.Class
                0x01, // Horse.Unk4
                0x02, // Horse.Unk5
                0x02, 0x00, // Horse.AvailableGrowthPoints
                
                // Horse.Unk7: An array of size 7. Each element has two 2 byte values
                0xD0, 0x07,
                0x3C, 0x00,
                
                0x1C, 0x02,
                0x00, 0x00,
                
                0xE8, 0x03,
                0x00, 0x00,
                
                0x00, 0x00,
                0x00, 0x00,
                
                0xE8, 0x03,
                0x1E, 0x00,
                
                0x0A, 0x00,
                0x0A, 0x00,
                
                0x0A, 0x00,
                0x00, 0x00,
                
                // More horse fields
                0x00,
                0x00, 0x00, 0x00, 0x00,
                0xE4, 0x67, 0xA1, 0xB8,
                0x02,
                0x00,
                0x7D, 0x2E, 0x03, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x04,
                0x00,
                0x00,
                0x00, 0x00,
                0x00, 0x00,
                0x01, 0x00,
                
                // Horse field: Horse Mastery Array of four 4 int values
                0xFE, 0x01, 0x00, 0x00, // speed booster/magic use
                0x21, 0x04, 0x00, 0x00, // jumps
                0xF8, 0x05, 0x00, 0x00, // sliding
                0xA4, 0xCF, 0x00, 0x00, // gliding, value here is divided by 10 by client
                
                0xE4, 0x67, 0xA1, 0xB8,
                0x00, 0x00, 0x00, 0x00,
                
                // Back to player fields
                0x01, // Equipment list size: List size, max 16 elements
                0x01, 0x00, 0x00, 0x00, // 4 byte - type
                0x31, 0x75, 0x00, 0x00, // 4 byte - TID
                0x01, 0x00, 0x00, 0x00, // 4 byte - ???
                0x01, 0x00, 0x00, 0x00, // 4 byte - ???
                
                // Buffer::ReadPlayerRelatedThing, shared with the structures in LoginOK
                0x00, 0x00, 0x00, 0x00,
                0x01,
                0x00, 0x00, 0x00, 0x00,
                0x00, // string
                0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, // Goes ignored?
                
                0x03, 0x00, // RANCH INDEX
                0x00,
                0x00,
                
                // Buffer::ReadAnotherPlayerRelatedSomething, also shared
                0x97, 0xA3, 0x79, 0x05, // Horse UID?
                0x12, 0x00, 0x00, 0x00,
                0xE4, 0x67, 0x6E, 0x01,
                
                // Buffer::ReadYetAnotherPlayerRelatedSomething, also shared
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, // string
                0x00, 0x00, 0x00, 0x00,
                
                0x00,
                0x00,


              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //unk1
              0x00, 0x00, 0x00, 0x00, //unk2
              0x00, 0x00, 0x00, 0x00, //unk3

              // unk4: structure with int, short, and something weird
              0x00, // list size, max 13 values

              0x00, //unk5
              0x00, 0x00, 0x00, 0x00, //unk6
              0x00, 0x00, 0x00, 0x00, //unk7 bitset, no idea what its for

              0x00, 0x00, 0x00, 0x00, //unk8
              0x00, 0x00, 0x00, 0x00, //unk9

              // unk10: list of three structures, each with Horse.TID, uint, byte, int, int, int, int, int
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 
              0x00, 
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,

              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 
              0x00, 
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,

              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 
              0x00, 
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,


              //unk11: structure
              0x01,
              0x01,

              0x00, 0x00, 0x00, 0x00 //unk12
          };
          this->send_command(response);
        } break;
      #endif

      #ifdef AcCmdCLGetMessengerInfo
      case AcCmdCLGetMessengerInfo:
        {
          DummyCommand response(AcCmdCLGetMessengerInfoOK);
          response.data = { 
            0x03, 0xBB, 0x2D, 0xD6, 
            0x7F, 0x00, 0x00, 0x01, // 127.0.0.1, messenger server IP
            0x31, 0x27, // port
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCRRanchSnapshot
      case AcCmdCRRanchSnapshot:
      {
        // Command consists of byte and byte array
        uint8_t unk0 = request.data[0];
        size_t unk1Size = request.data.size() - 1;
        uint8_t* unk1 = request.data.data()+1;

        DummyCommand response(AcCmdCRRanchSnapshotNotify);
        response.data.push_back(0x03); // player 2 ranch index
        response.data.push_back(0x00);
        response.data.push_back(unk0);
        for(size_t i = 0; i < unk1Size; ++i)
          response.data.push_back(unk1[i]);
        
        for (auto& [id, client]: server.clients) {
          if(&client != this)
          {
            client.send_command(response);
          }
        }
      }
      break;
      #endif

      #ifdef AcCmdCRRanchCmdAction
      case AcCmdCRRanchCmdAction:
      {
        DummyCommand response(AcCmdCRRanchCmdActionNotify);
        response.data = {

          0x02, 0x00, // 2 bytes, unknown
          0x03, 0x00, // 2 bytes, unknown

          0x01, 0x00, 0x00, 0x00,   // 4 bytes, unknown

          0x01, // 1 byte, unknown
        };
        this->send_command(response);
      }
      break;
      #endif

      #ifdef AcCmdCREmblemList
      case AcCmdCREmblemList:
      {
        DummyCommand response(AcCmdCREmblemListOK);
        response.data = {
          0x03, // list size, up to 64 (< 0x41)

          // struct of 2-byte values
          // struct for which emblems are unlocked
          // if first value is 0x03,
          // 'first' unlocked emblem should be the third one
          0x03, 0x00,
          0x05, 0x00,
          0x07, 0x00,
        };
        this->send_command(response);
      }
      break;
      #endif

      #ifdef AcCmdCRUpdateBusyState
      case AcCmdCRUpdateBusyState:
        {
          // Request encodes one byte only supposedly even though i get more bytes in the buffer
          uint8_t state = request.data[0];

          DummyCommand response(AcCmdCRUpdateBusyStateNotify);
          response.data = {
            0xE8, 0xE2, 0x06, 0x00, // Self UID (confirmed)
            state
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCRLeaveRanch
      case AcCmdCRLeaveRanch:
        {
          DummyCommand response(AcCmdCRLeaveRanchOK);
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCREnterRoom
      case AcCmdCREnterRoom:
        {
          DummyCommand response(AcCmdCREnterRoomOK);
          response.data = {
            // List of racers, consists of a list of structures with:
            // byte 
            // byte 
            // int 
            // int 
            // int 
            // string
            // byte
            // uint
            // byte
            // byte (isNPC?)
            // if the last byte is zero:
            //   a list (with byte length) of AcCmdCLLoginOK_Unk3_Element
            //   AcCmdCLLoginOK_Unk15
            //   Horse
            //   int
            // if not:
            //   uint
            // a structure with byte and AnotherPlayerRelatedSomething
            // YetAnotherPlayerRelatedSomething
            // PlayerRelatedThing
            // RanchUnk11
            // byte
            // byte
            // byte
            // byte
            0x01, 0x00, 0x00, 0x00, // list size (yes, 4 bytes this time)
              0x00,
              0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00,
              'r', 'g', 'n', 't', 0x00, // racer name
              0x00,
              0x00, 0x00, 0x00, 0x00,
              0x00,
              0x00, // isNPC = false

              0x01, // Equipment list size: List size, max 16 elements
                0x01, 0x00, 0x00, 0x00, // 4 byte - type
                0x31, 0x75, 0x00, 0x00, // 4 byte - TID
                0x01, 0x00, 0x00, 0x00, // 4 byte - ???
                0x01, 0x00, 0x00, 0x00, // 4 byte - ???

              // Unk15: Another small structure
              0x0A,
              0x00,
              0x00,
              0x01,
              0x01, 0x00,
              0x04, 0x00,
              0x08, 0x00,
              0x08, 0x00,
              0x08, 0x00,
              0x00, 0x00,
              
              // Horse: Big ass structure now, probably horse info
              // Horse.TIDs
              0x96, 0xA3, 0x79, 0x05, // Horse.TIDs.MountTID Unique horse identifier
              0x21, 0x4E, 0x00, 0x00, // Horse.TIDs.HorseTID Horse model
              /* Horse name: "idontunderstand" */ 0x69, 0x64, 0x6F, 0x6E, 0x74, 0x75, 0x6E, 0x64, 0x65, 0x72, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x00,
              // Horse.Appearance: Structure. Probably horse appearance
              0x02,
              0x03,
              0x03,
              0x03,
              0x04,
              0x04,
              0x05,
              0x03,
              0x04,
              // Horse.Stats
              0x04, 0x00, 0x00, 0x00, // agility
              0x03, 0x00, 0x00, 0x00, // spirit
              0x02, 0x00, 0x00, 0x00, // speed
              0x01, 0x00, 0x00, 0x00, // strength 
              0x13, 0x00, 0x00, 0x00, // control

              0x00, 0x00, 0x00, 0x00, // Horse.Rating
              0x15, // Horse.Class
              0x01, // Horse.Unk4
              0x02, // Horse.Unk5
              0x02, 0x00, // Horse.AvailableGrowthPoints

              // Horse.Unk7: An array of size 7. Each element has two 2 byte values
              0xD0, 0x07,
              0x3C, 0x00,

              0x1C, 0x02,
              0x00, 0x00,

              0xE8, 0x03,
              0x00, 0x00,

              0x00, 0x00,
              0x00, 0x00,

              0xE8, 0x03,
              0x1E, 0x00,

              0x0A, 0x00,
              0x0A, 0x00,

              0x0A, 0x00,
              0x00, 0x00,

              // More horse fields
              0x00,
              0x00, 0x00, 0x00, 0x00, 
              0xE4, 0x67, 0xA1, 0xB8, 
              0x02, 
              0x00, 
              0x7D, 0x2E, 0x03, 0x00,
              0x00, 0x00, 0x00, 0x00, 
              0x00, 
              0x00, 
              0x00, 
              0x00, 
              0x04, 
              0x00,
              0x00,
              0x00, 0x00,
              0x00, 0x00,
              0x01, 0x00, 

              // Horse field: Horse Mastery Array of four 4 int values
              0xFE, 0x01, 0x00, 0x00, // speed booster/magic use
              0x21, 0x04, 0x00, 0x00, // jumps
              0xF8, 0x05, 0x00, 0x00, // sliding
              0xA4, 0xCF, 0x00, 0x00, // gliding, value here is divided by 10 by client
              
              // still horse
              0xE4, 0x67, 0xA1, 0xB8, 
              0x00, 0x00, 0x00, 0x00, 
            
              // last uint
              0x00, 0x00, 0x00, 0x00,

              0x00,
              // Unk26: Buffer::ReadAnotherPlayerRelatedSomething, also shared
              0x96, 0xA3, 0x79, 0x05, // Horse UID?
              0x12, 0x00, 0x00, 0x00,             
              0xE4, 0x67, 0x6E, 0x01, 

              // Buffer::ReadYetAnotherPlayerRelatedSomething, also shared
              0x00, 0x00, 0x00, 0x00, 
              0x00, 0x00, 0x00, 0x00, 
              0x00, // string
              0x00, 0x00, 0x00, 0x00,

              // Unk24: Buffer::ReadPlayerRelatedThing, shared with the structures in EnterRanch
              0x00, 0x00, 0x00, 0x00, 
              0x01, 
              0x00, 0x00, 0x00, 0x00, 
              0x00, // string
              0x00, 
              0x00, 0x00, 0x00, 0x00,            
              0x00, // Goes ignored?

              //unk11: structure
              0x01,
              0x01,

              // those last 4 bytes
              0x00,
              0x00,
              0x00,
              0x00,


            0x00,
            0x00, 0x00, 0x00, 0x00,

            // Structure: Room description
            'T', 'h', 'e', ' ', 'R', 'o', 'o', 'm', 0x00, // string
            0x01,
            'S', 'i', 'l', 'e', 'n', 't', ' ', 'H', 'i', 'l', 'l', ' ', '4', 0x00, // string
            0x01,
            0x01,
            0x01, 0x00,
            0x01,
            0x01, 0x00,
            0x01,
            0x01,

            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,

            // unk9: structure that depends on this+0x2980 == 2 (inside unk3?)
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00,
            // list containing ints
            0x00, // list size

            0x00, 0x00, 0x00, 0x00,
            // something weird in the decompilation in here
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCRChangeRoomOptions
      case AcCmdCRChangeRoomOptions:
        {
          // Request consists of short, byte, byte, short, byte
          // Response consists of: short as a bitfield
          //  if & 1 != 0: string
          //  if & 2 != 0: byte
          //  if & 4 != 0: string
          //  if & 8 != 0: byte
          //  if  & 16 != 0: short
          //  if  & 32 != 0: byte
          // (same values as AcCmdCREnterRoomOK.Unk3?)
          DummyCommand response(AcCmdCRChangeRoomOptionsNotify);
          response.data = { 0x00, 0x00 };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCRStartRace
      case AcCmdCRStartRace:
        {
          // Request consists of a list (length specified in a byte) of shorts

          DummyCommand response(AcCmdCRStartRaceNotify);
          response.data = {
            0x00,
            0x00,
            0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00,

            // list of short string byte byte short int short int
            0x00,  // list size

            127, 0, 0, 1, // ip
            0x32, 0x2f, // Port

            0x00,

            // unk9: structure
            0x00, 0x00,
            0x00,
            0x00,
            0x00, 0x00, 0x00, 0x00,
            // unk9.unk5: list of ints
            0x00, // list size
            // if this+8 == 3?
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00,
            0x00, 0x00, 0x00, 0x00,

            // unk10: another structure
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,

            0x00, 0x00,
            0x00,

            // unk13: structure
            0x00,
            0x00, 0x00, 0x00, 0x00,
            // list of ints
            0x00, // list size

            0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00,
            // unk18: list of short and a nested list of int
            0x00, // list size
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdUserRaceTimer
      case AcCmdUserRaceTimer:
        {
          // Request contains a long, i assume with the time

          // Response contains two longs
          DummyCommand response(AcCmdUserRaceTimerOK);
          response.data = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdCRLoadingComplete
      case AcCmdCRLoadingComplete:
        {
          // Request contains nothing
          // Response contains short
          DummyCommand response(AcCmdCRLoadingCompleteNotify);
          response.data = { 
            0x00, 0x01 // index of the player who finished loading?
          };
          this->send_command(response);
        }
        break;
      #endif

      #ifdef AcCmdRCMissionEvent
      case AcCmdRCMissionEvent:
      {
        DummyCommand response(AcCmdRCMissionEvent);
        response.data = request.data;
        this->send_command(response);
      }
      break;
      #endif

      #ifdef AcCmdCRBreedingWishlist
      case AcCmdCRBreedingWishlist:
      {
        DummyCommand response(AcCmdCRBreedingWishlistOK);
        response.data = {
          0x00, // size
        };
        this->send_command(response);
      }
      break;
      #endif

      #ifdef AcCmdCRSearchStallion
      case AcCmdCRSearchStallion:
      {
        DummyCommand response(AcCmdCRSearchStallionOK);
        response.data = {
          0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00,

          // Horses to breed with
          0x01, // count, max 11 elements

          't', 'e', 's', 't', 0x00, // max 17 chars
          0x21, 0x4E, 0x00, 0x03,
          0x21, 0x4E, 0x00, 0x00,
          'i', 'u', 'n', 'd', 'e', 'r', 's', 't', 'a', 'n', 'd', 0x00, // max 17 chars
          0x04, // Grade
          0x00, // Chance
          0x01, 0x00, 0x00, 0x00, // Price
          0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF,

          // Stats (abilities) (MountMultiAbility)
          0x09, 0x00, 0x00, 0x00, // .Agility
          0x09, 0x00, 0x00, 0x00, // spirit
          0x09, 0x00, 0x00, 0x00, // speed
          0x09, 0x00, 0x00, 0x00, // strength
          0x09, 0x00, 0x00, 0x00, // .Ambition

          // Appearance (MountPartSet)
          0x01, // .SkinId (MountSkinInfo)
          0x04, // .ManeId (MountManeInfo)
          0x04, // .TailId (MountTailInfo)
          0x05, // .FaceId (MountFaceInfo)
          0x00, // .Fig_Scale
          0x00, // .Fig_LegLength
          0x00, // .Fig_LegVol
          0x00, // .Fig_BodyLength
          0x00, // .Fig_BodyVol

          0x05,
          0x00, // Coat bonus
        };
        this->send_command(response);
      }
      break;
      #endif

      #ifdef AcCmdCREnterBreedingMarket
      case AcCmdCREnterBreedingMarket:
      {
        DummyCommand response(AcCmdCREnterBreedingMarketOK);
        response.data = {
          0x01, // List of horses

          // User horses available for breeding
          0x96, 0xA3, 0x79, 0x05, // Horse.TIDs.HorseUID Unique horse identifier
          0x21, 0x4E, 0x00, 0x00, // Horse.TIDs.MountTID Horse model identifier
          0x00,
          0x00, 0x00, 0x00, 0x00,
          0x00,
          0x00,
        };
        this->send_command(response);
      }
      break;
      #endif

      #ifdef AcCmdCRTryBreeding
      case AcCmdCRTryBreeding:
      {
        // mare UID
        // stalion UID

        DummyCommand response(AcCmdCRTryBreedingOK);
        response.data = {

          0x96, 0xA3, 0x79, 0xFF, // Horse.TIDs.HorseUID Unique horse identifier
          0x21, 0x4E, 0x00, 0x00, // Horse.TIDs.MountTID Horse model identifier
          0x00, 0x00, 0x00, 0x00, // val
          0x00, 0x00, 0x00, 0x00, // count

          0x00,

          // Appearance (MountPartSet)
          0x01, // .SkinId (MountSkinInfo)
          0x04, // .ManeId (MountManeInfo)
          0x04, // .TailId (MountTailInfo)
          0x05, // .FaceId (MountFaceInfo)
          0x00, // .Fig_Scale
          0x00, // .Fig_LegLength
          0x00, // .Fig_LegVol
          0x00, // .Fig_BodyLength
          0x00, // .Fig_BodyVol

          // Stats (abilities) (MountMultiAbility)
          0x09, 0x00, 0x00, 0x00, // .Agility
          0x09, 0x00, 0x00, 0x00, // spirit
          0x09, 0x00, 0x00, 0x00, // speed
          0x09, 0x00, 0x00, 0x00, // strength
          0x09, 0x00, 0x00, 0x00, // .Ambition


          0x00, 0x00, 0x00, 0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00, 0x00,
          0x00,

        };
        this->send_command(response);
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
        if(!mute)
        {
          std::cout << "WARNING! Packet " << GetMessageName(message_magic.id) << " (0x" << std::hex << message_magic.id << std::dec << ") not handled" << std::endl << std::endl;
        }
        break;
    }
    
    read_loop(server);
  });
}

void alicia::Client::rotate_xor_key()
{
  this->xor_key = this->xor_key * xor_multiplier + xor_control;
}

void alicia::Client::send_command(alicia::ICommand& cmd)
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
    std::cout << ">>> SEND " << this->_socket.remote_endpoint().address().to_string() << ":" << this->_socket.remote_endpoint().port() << " ";
    cmd.Log();
  }

  std::vector<uint8_t> cmdContents = cmd.AsBytes();

  // gametree forgot to encode clientbound packets

  uint16_t totalPacketSize = sizeof(uint32_t) + cmdContents.size(); // size of the magic header + size of the packet contents
  uint32_t responseEncodedMagic = alicia::encode_message_magic({cmd.GetCommandId(), totalPacketSize});
  this->_socket.write_some(boost::asio::const_buffer(&responseEncodedMagic, sizeof(uint32_t)));
  this->_socket.write_some(boost::asio::const_buffer(cmdContents.data(), cmdContents.size()));
}

void alicia::Server::host(short port)
{
  asio::ip::tcp::endpoint server_endpoint(asio::ip::tcp::v4(), port);
  printf("Hosting the server on port %d\n", port);
  _acceptor.open(server_endpoint.protocol());
  _acceptor.bind(server_endpoint);
  _acceptor.listen();
  accept_loop();
}

void alicia::Server::accept_loop()
{
  _acceptor.async_accept([&](boost::system::error_code error, asio::ip::tcp::socket client_socket) {
    printf("+++ CONN %s:%d\n\n", client_socket.remote_endpoint().address().to_string().c_str(), client_socket.remote_endpoint().port());
    const auto [itr, _] = clients.emplace(client_id++, std::move(client_socket));
    itr->second.read_loop(*this);

    accept_loop();
  });
}