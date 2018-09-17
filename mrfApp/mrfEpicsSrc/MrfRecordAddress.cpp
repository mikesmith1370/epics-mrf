/*
 * Copyright 2015 aquenos GmbH.
 * Copyright 2015 Karlsruhe Institute of Technology.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * This software has been developed by aquenos GmbH on behalf of the
 * Karlsruhe Institute of Technology's Institute for Beam Physics and
 * Technology.
 *
 * This software contains code originally developed by aquenos GmbH for
 * the s7nodave EPICS device support. aquenos GmbH has relicensed the
 * affected poritions of code from the s7nodave EPICS device support
 * (originally licensed under the terms of the GNU GPL) under the terms
 * of the GNU LGPL version 3 or newer.
 */

#include <algorithm>
#include <cctype>
#include <climits>
#include <stdexcept>
#include <tuple>

#include "MrfRecordAddress.h"

namespace anka {
namespace mrf {
namespace epics {

namespace {

const std::string emptyString;

bool compareStringsIgnoreCase(const std::string &str1,
    const std::string &str2) {
  if (str1.length() != str2.length()) {
    return false;
  }
  return std::equal(str1.begin(), str1.end(), str2.begin(),
      [](char c1, char c2) {return std::tolower(c1) == std::tolower(c2);});
}

std::pair<std::size_t, std::size_t> findNextToken(const std::string &str,
    const std::string &delimiters, std::size_t startPos) {
  if (str.length() == 0) {
    return std::make_pair(std::string::npos, 0);
  }
  std::size_t startOfToken = str.find_first_not_of(delimiters, startPos);
  if (startOfToken == std::string::npos) {
    return std::make_pair(std::string::npos, 0);
  }
  std::size_t endOfToken = str.find_first_of(delimiters, startOfToken);
  if (endOfToken == std::string::npos) {
    return std::make_pair(startOfToken, str.length() - startOfToken);
  }
  return std::make_pair(startOfToken, endOfToken - startOfToken);
}

std::tuple<std::uint32_t, signed char, signed char> parseMemoryAddress(
    const std::string &addressString, MrfRecordAddress::DataType dataType) {
  // We use the stoul function for converting. This only works, if the unsigned
  // long data-type is large enough (which it should be on virtually all
  // platforms).
  static_assert(sizeof(unsigned long) >= sizeof(std::uint32_t), "unsigned long data-type is not large enough to hold a uint32_t");
  signed char maxBitIndex;
  switch (dataType) {
  case MrfRecordAddress::DataType::uInt16:
    maxBitIndex = 15;
    break;
  case MrfRecordAddress::DataType::uInt32:
    maxBitIndex = 31;
    break;
  }
  std::size_t numberLength;
  unsigned long address;
  try {
    address = std::stoul(addressString, &numberLength, 0);
  } catch (std::invalid_argument&) {
    throw std::invalid_argument(
        std::string("Invalid memory address in record address: ")
            + addressString);
  } catch (std::out_of_range&) {
    throw std::invalid_argument(
        std::string("Invalid memory address in record address: ")
            + addressString);
  }
  if (address > UINT32_MAX) {
    throw std::invalid_argument(
        std::string("Invalid memory address in record address: ")
            + addressString);
  }
  if (numberLength == addressString.length()) {
    return std::make_tuple(static_cast<std::uint32_t>(address), maxBitIndex, 0);
  }
  if (addressString[numberLength] != '[') {
    throw std::invalid_argument(
        std::string("Invalid memory address in record address: ")
            + addressString + ". Expected '[' but found '"
            + addressString[numberLength] + "'.");
  }
  if (addressString[addressString.length() - 1] != ']') {
    throw std::invalid_argument(
        std::string("Invalid memory address in record address: ")
            + addressString + ". Expected ']' but found '"
            + addressString[addressString.length() - 1] + "'.");
  }
  std::string bitIndexString = addressString.substr(numberLength + 1,
      addressString.length() - numberLength - 2);
  int bitIndex;
  try {
    bitIndex = std::stoi(bitIndexString, &numberLength, 0);
  } catch (std::invalid_argument&) {
    throw std::invalid_argument(
        std::string("Invalid bit index in record address: ") + addressString);
  } catch (std::out_of_range&) {
    throw std::invalid_argument(
        std::string("Invalid bit index in record address: ") + addressString);
  }
  if (bitIndex < 0 || bitIndex > maxBitIndex) {
    throw std::invalid_argument(
        std::string("Invalid bit index in record address: ") + addressString);
  }
  signed char highBitIndex = static_cast<signed char>(bitIndex);
  if (numberLength == bitIndexString.length()) {
    return std::make_tuple(static_cast<std::uint32_t>(address), highBitIndex,
        highBitIndex);
  }
  bitIndexString = bitIndexString.substr(numberLength + 1, std::string::npos);
  try {
    bitIndex = std::stoi(bitIndexString, &numberLength, 0);
  } catch (std::invalid_argument&) {
    throw std::invalid_argument(
        std::string("Invalid bit index in record address: ") + addressString);
  } catch (std::out_of_range&) {
    throw std::invalid_argument(
        std::string("Invalid bit index in record address: ") + addressString);
  }
  if (bitIndex < 0 || bitIndex > maxBitIndex) {
    throw std::invalid_argument(
        std::string("Invalid bit index in record address: ") + addressString);
  }
  signed char lowBitIndex = static_cast<signed char>(bitIndex);
  if (highBitIndex < lowBitIndex) {
    throw std::invalid_argument(
        std::string("Invalid bit index in record address: ") + addressString
            + ". The index of the highest bit must not be less than the index of the lowest bit.");
  }
  return std::make_tuple(static_cast<std::uint32_t>(address), highBitIndex,
      lowBitIndex);
}

}

MrfRecordAddress::MrfRecordAddress(const std::string &addressString) :
    elementDistance(0), zeroOtherBits(false), verify(true), readOnInit(true), changedElementsOnly(
        false) {
  const std::string delimiters(" \t\n\v\f\r");
  std::size_t tokenStart, tokenLength;
  // First, read the device name.
  std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
      0);
  if (tokenStart == std::string::npos) {
    throw std::invalid_argument("Could not find device ID in record address.");
  }
  deviceId = addressString.substr(tokenStart, tokenLength);
  // Next, read the address. However, we have to delay the parsing of the address
  // until we know the data type.
  std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
      tokenStart + tokenLength);
  if (tokenStart == std::string::npos) {
    throw std::invalid_argument(
        "Could not find memory address in record address.");
  }
  std::string memoryAddressString = addressString.substr(tokenStart,
      tokenLength);
  // Next, read the data-type.
  std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
      tokenStart + tokenLength);
  if (tokenStart == std::string::npos) {
    throw std::invalid_argument("Could not find data type in record address.");
  }
  std::string dataTypeString = addressString.substr(tokenStart, tokenLength);
  if (compareStringsIgnoreCase(dataTypeString, "uint16")) {
    dataType = DataType::uInt16;
  } else if (compareStringsIgnoreCase(dataTypeString, "uint32")) {
    dataType = DataType::uInt32;
  } else {
    throw std::invalid_argument(
        std::string("Invalid data-type in record address: ") + dataTypeString);
  }
  // Now we know the data type, so we can parse the memory address.
  std::tie(address, highestBit, lowestBit) = parseMemoryAddress(
      memoryAddressString, dataType);
  // Read additional optional flags.
  std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
      tokenStart + tokenLength);
  const std::string elementDistanceString = "element_distance=";
  while (tokenStart != std::string::npos) {
    std::string token = addressString.substr(tokenStart, tokenLength);
    if (compareStringsIgnoreCase(token, "zero_other_bits")) {
      zeroOtherBits = true;
    } else if (compareStringsIgnoreCase(token, "no_verify")) {
      verify = false;
      readOnInit = false;
    } else if (compareStringsIgnoreCase(token, "no_read_on_init")) {
      readOnInit = false;
    } else if (compareStringsIgnoreCase(token, "changed_elements_only")) {
      changedElementsOnly = true;
    } else if (token.length() >= elementDistanceString.length()
        && compareStringsIgnoreCase(
            token.substr(0, elementDistanceString.length()),
            elementDistanceString)) {
      std::size_t numberLength;
      unsigned long elementDistance;
      try {
        elementDistance = std::stoul(
            token.substr(elementDistanceString.length(), std::string::npos),
            &numberLength, 0);
      } catch (std::invalid_argument&) {
        throw std::invalid_argument(
            std::string("Invalid element distance in record address: ")
                + token);
      } catch (std::out_of_range&) {
        throw std::invalid_argument(
            std::string("Invalid element distance in record address: ")
                + token);
      }
      // We have to make sure that the value fits into a signed integer.
      if (elementDistance > INT_MAX) {
        throw std::invalid_argument(
            std::string("Invalid element distance in record address: ")
                + token);
      }
      this->elementDistance = elementDistance;
    } else {
      throw std::invalid_argument(
          std::string("Unrecognized token in record address: ") + token);
    }
    std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
        tokenStart + tokenLength);
  }
}

}
}
}
