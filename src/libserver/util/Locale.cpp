//
// Created by rgnter on 13/07/2025.
//

#include "libserver/util/Locale.hpp"

#ifndef WIN32
#include <unicode/ucnv.h>
#elifdef WIN32
#include <icu.h>
#endif

#include <format>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace server
{

namespace locale
{

std::string ToUtf8(const std::string& input)
{
  std::string output;

  std::u16string unicodeOutput;
  unicodeOutput.resize(input.length() * 2);

  thread_local UErrorCode error;

  thread_local UConverter* koreanConv = ucnv_open("EUC-KR", &error);
  ucnv_toUChars(
    koreanConv,
    unicodeOutput.data(),
    unicodeOutput.size(),
    input.data(),
    input.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    if (error == U_FILE_ACCESS_ERROR)
    {
      spdlog::error("Unicode ICU data for conversion from EUC-KR not available");
    }

    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from EUC-KR to Unicode: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  thread_local UConverter* utfConv = ucnv_open("UTF-8", &error);
  output.resize(UCNV_GET_MAX_BYTES_FOR_STRING(
    unicodeOutput.length(),
    ucnv_getMaxCharSize(utfConv)));

  ucnv_fromUChars(
    utfConv,
    output.data(),
    output.size(),
    unicodeOutput.data(),
    unicodeOutput.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from Unicode to UTF-8: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  return {output.data()};
}

std::string FromUtf8(const std::string& input)
{
  std::string output;

  std::u16string unicodeOutput;
  unicodeOutput.resize(input.length() * 2);

  thread_local UErrorCode error;

  thread_local UConverter* utfConv = ucnv_open("UTF-8", &error);
  ucnv_toUChars(
    utfConv,
    unicodeOutput.data(),
    unicodeOutput.size(),
    input.data(),
    input.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from UTF-8 to Unicode: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  thread_local UConverter* koreanConv = ucnv_open("EUC-KR", &error);
  output.resize(UCNV_GET_MAX_BYTES_FOR_STRING(
    unicodeOutput.length(),
    ucnv_getMaxCharSize(koreanConv)));

  ucnv_fromUChars(
    koreanConv,
    output.data(),
    output.size(),
    unicodeOutput.data(),
    unicodeOutput.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    if (error == U_FILE_ACCESS_ERROR)
    {
      spdlog::error("Unicode ICU data for conversion to EUC-KR not available");
    }

    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from Unicode to EUC-KR: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  return {output.data()};
}

} // namespace locale

} // namespace server