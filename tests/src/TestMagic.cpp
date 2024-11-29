#include "libserver/Alicia.hpp"
#include "libserver/command/CommandProtocol.hpp"

#include <cassert>

namespace {

  //! Perform test of magic encoding/decoding.
  void TestMagic()
  {
    const alicia::MessageMagic magic {
      .id = 7,
      .length = 29
    };

    // Test encoding of the magic.
    const auto encoded_magic = alicia::encode_message_magic(magic);
    assert(encoded_magic == 0x8D06CD01);

    // Test decoding of the magic.
    const auto decoded_magic = alicia::decode_message_magic(encoded_magic);
    assert(decoded_magic.id == magic.id);
    assert(decoded_magic.length == magic.length);
  }

} // namespace anon

int main() {
  TestMagic();
}

