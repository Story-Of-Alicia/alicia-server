//
// Created by rgnter on 13/07/2025.
//

#include "libserver/util/Locale.hpp"

#include "libserver/util/Deferred.hpp"

#ifdef _MSC_VER
#include <icu.h>
#else
#include <unicode/ucnv.h>
#include <unicode/uregex.h>
#endif

#include <format>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace server
{

namespace locale
{

namespace
{

constexpr size_t EucKrWideByteCount = 2;
constexpr size_t EucKrNarrowByteCount = 1;

constexpr size_t MinLatinLetterCount = 3;
constexpr size_t MinKoreanLetterCount = 2;

constexpr std::u16string_view KoreanLettersPattern = u"[가-힣]";
constexpr std::u16string_view LatinLettersPattern = u"[A-Za-z0-9._-]";
constexpr std::u16string_view ValidLettersPattern = u"[^가-힣A-Za-z0-9._-]";

} // anon namespace

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


bool IsNameValid(
  const std::string& input,
  const size_t maxStringByteCapacity)
{
  if (input.empty())
    return false;

  UErrorCode status{U_ZERO_ERROR};

  // Create the unicode text from the input.
  UText inputString = UTEXT_INITIALIZER;
  utext_openUTF8(&inputString, input.data(), static_cast<int64_t>(input.length()), &status);
  if (U_FAILURE(status))
    throw std::runtime_error("Failed to create utext from input");

  // Defer close of the input string.
  Deferred closeInputString([&inputString]()
  {
    utext_close(&inputString);
  });

  thread_local auto* validLettersRegex = uregex_open(
    ValidLettersPattern.data(),
    ValidLettersPattern.length(),
    0,
    nullptr,
    &status);
  if (U_FAILURE(status))
    throw std::runtime_error("Failed to create valid letters regex");

  // Provide the input string to the valid letters regex.
  uregex_setUText(validLettersRegex, &inputString, &status);
  if (U_FAILURE(status))
    throw std::runtime_error("Failed to set valid letters regex text input");

  // Defer close of the thread local valid letters regex.
  thread_local Deferred closeValidLettersRegex([]()
  {
    if (validLettersRegex)
      uregex_close(validLettersRegex);
  });

  // Validate the letters in the input string.
  const bool hasInvalidLetters = uregex_find(validLettersRegex, -1, &status);
  if (U_SUCCESS(status) && hasInvalidLetters)
    return false;

  // Lambda counts the number of regex matches.
  const auto countRegexMatches = [](URegularExpression* regex)
  {
    size_t count{0};
    UErrorCode status;
    while (uregex_findNext(regex, &status) && U_SUCCESS(status))
    {
      count++;
    }

    return count;
  };

  thread_local auto* koreanLettersRegex = uregex_open(
    KoreanLettersPattern.data(),
    KoreanLettersPattern.length(),
    0,
    nullptr,
    &status);
  if (U_FAILURE(status))
    throw std::runtime_error("Failed to create korean letters regex");

  // Provide the input string to the korean letters regex.
  uregex_setUText(koreanLettersRegex, &inputString, &status);
  if (U_FAILURE(status))
    throw std::runtime_error("Failed to set korean letters regex text input");

  // Defer close of the thread local korean letters regex.
  thread_local  Deferred closeKoreanLettersRegex([]()
  {
    if (koreanLettersRegex)
      uregex_close(koreanLettersRegex);
  });

  thread_local auto* latinLettersRegex = uregex_open(
    LatinLettersPattern.data(),
    LatinLettersPattern.length(),
    0,
    nullptr,
    &status);
  if (U_FAILURE(status))
    throw std::runtime_error("Failed to create latin letters regex");

  // Provide the input string to the latin letters regex.
  uregex_setUText(latinLettersRegex, &inputString, &status);
  if (U_FAILURE(status))
    throw std::runtime_error("Failed to set latin letters regex text input");

  // Defer close of the thread local latin letters regex.
  thread_local Deferred closeLatinLettersRegex([]()
  {
    if (latinLettersRegex)
      uregex_close(latinLettersRegex);
  });

  const size_t koreanLetterCount = countRegexMatches(koreanLettersRegex);
  const size_t latinLetterCount = countRegexMatches(latinLettersRegex);

  // Determine the max length of the input string.
  // Max length is determined from the actual byte capacity of the input string. Not the codepoints.

  // Calculate the byte count of the input string.
  // Count the bytes used to represent wide codepoints and narrow code points.
  const size_t inputStringByteCount = koreanLetterCount * EucKrWideByteCount + latinLetterCount * EucKrNarrowByteCount;
  if (inputStringByteCount > maxStringByteCapacity)
    return false;

  // Determine the min length of the input string.
  // Min length is determined by the count of codepoints.

  // todo: technical limitation, all arabic numbers are considered to be latin
  //       and thus korean names with numbers are not considered pure.
  const bool isPureKorean = latinLetterCount == 0 && koreanLetterCount > 0;
  const size_t minLetterCount = isPureKorean
    ? MinKoreanLetterCount
    : MinLatinLetterCount;

  // Get the code point count of the input string.
  const auto inputStringLength = utext_nativeLength(&inputString);
  if (inputStringLength < minLetterCount)
    return false;

  return true;
}

} // namespace locale

} // namespace server