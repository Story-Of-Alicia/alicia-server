//
// Created by rgnter on 13/07/2025.
//

#include "libserver/util/Locale.hpp"

#ifndef WIN32
#include <iconv.h>
#include <errno.h>
#elifdef WIN32
#include <icu.h>
#endif

#include <stdexcept>

namespace
{

#ifndef WIN32
//! Invalid iconv object.
const iconv_t InvalidIconv = reinterpret_cast<iconv_t>(-1);
#endif

} // anon namespace

namespace server
{

namespace locale
{

const std::string ToUtf8(const std::string& input)
{
  std::string output;
  output.resize(input.length() * 2);

  #ifndef WIN32
  const thread_local auto icd = iconv_open("EUC-KR", "UTF-8");
  if (icd == InvalidIconv)
    throw std::runtime_error("Conversion from EUC-KR to UTF-8 not supported");

  char const* source = input.data();
  size_t sourceBytesAvailable = input.length();
  char* destination = output.data();
  size_t destinationBytesAvailable = output.size();

  iconv(icd,
    const_cast<char**>(&source), // incorrect API specification, underlying data are const
    &sourceBytesAvailable,
    &destination,
    &destinationBytesAvailable);

  iconv_close(icd);
  #else

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

  thread_local UConverter* utfConf = ucnv_open("UTF-8", &error);
  ucnv_fromUChars(
    utfConf,
    output.data(),
    output.size(),
    unicodeOutput.data(),
    unicodeOutput.length(),
    &error);
  #endif

  return std::string(output.data());
}

const std::string FromUtf8(const std::string& input)
{
  std::string output;
  output.resize(input.length() * 2);

  #ifndef WIN32
  const thread_local auto icd = iconv_open("UTF-8", "EUC-KR");
  if (icd == InvalidIconv)
    throw std::runtime_error("Conversion from UTF-8 to EUC-KR not supported");

  char const* source = input.data();
  size_t sourceBytesAvailable = input.length();
  char* destination = output.data();
  size_t destinationBytesAvailable = output.size();

  iconv(icd,
    const_cast<char**>(&source), // incorrect API specification, underlying data are const
    &sourceBytesAvailable,
    &destination,
    &destinationBytesAvailable);

  iconv_close(icd);
  #else

  std::u16string unicodeOutput;
  unicodeOutput.resize(input.length() * 2);

  thread_local UErrorCode error;

  thread_local UConverter* koreanConv = ucnv_open("UTF-8", &error);
  ucnv_toUChars(
    koreanConv,
    unicodeOutput.data(),
    unicodeOutput.size(),
    input.data(),
    input.length(),
    &error);

  thread_local UConverter* utfConf = ucnv_open("EUC-KR", &error);
  ucnv_fromUChars(
    utfConf,
    output.data(),
    output.size(),
    unicodeOutput.data(),
    unicodeOutput.length(),
    &error);
  #endif

  return std::string(output.data());
}

} // namespace locale

} // namespace server