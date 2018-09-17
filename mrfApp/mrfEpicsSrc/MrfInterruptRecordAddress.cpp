/*
 * Copyright 2016 aquenos GmbH.
 * Copyright 2016 Karlsruhe Institute of Technology.
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
#include <string>
#include <tuple>
#include <utility>

#include "MrfInterruptRecordAddress.h"

namespace anka {
namespace mrf {
namespace epics {

// We put all locally used functions into an anonoymous namespace so that they
// do not collide with other functions that might accidentally have the same
// name.
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

} // anonymous namespace

MrfInterruptRecordAddress::MrfInterruptRecordAddress(
    const std::string &addressString) :
    deviceId(""), interruptFlagsMask(0xffffffff) {
  const std::string delimiters(" \t\n\v\f\r");
  std::size_t tokenStart, tokenLength;
  // First, read the device name.
  std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
      0);
  if (tokenStart == std::string::npos) {
    throw std::invalid_argument("Could not find device ID in record address.");
  }
  this->deviceId = addressString.substr(tokenStart, tokenLength);
  // Read additional optional flags.
  std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
      tokenStart + tokenLength);
  const std::string interruptFlagsMaskString = "interrupt_flags_mask=";
  while (tokenStart != std::string::npos) {
    std::string token = addressString.substr(tokenStart, tokenLength);
    if (token.length() >= interruptFlagsMaskString.length()
        && compareStringsIgnoreCase(
            token.substr(0, interruptFlagsMaskString.length()),
            interruptFlagsMaskString)) {
      std::size_t numberLength;
      unsigned long interruptFlagsMask;
      // We use the stoul function for converting. This only works, if the
      // unsigned long data-type is large enough (which it should be on
      // virtually all platforms).
      static_assert(sizeof(unsigned long) >= sizeof(std::uint32_t), "unsigned long data-type is not large enough to hold a uint32_t");
      try {
        interruptFlagsMask = std::stoul(
            token.substr(interruptFlagsMaskString.length(), std::string::npos),
            &numberLength, 0);
      } catch (std::invalid_argument&) {
        throw std::invalid_argument(
            std::string("Invalid interrupt flags mask in record address: ")
                + token);
      } catch (std::out_of_range&) {
        throw std::invalid_argument(
            std::string("Invalid interrupt flags mask in record address: ")
                + token);
      }
      // The interrupts flags mask must fit into a 32-bit unsigned integer and
      // it must not be zero (a zero mask would mean that the record is never
      // processed, so it does not make any sense).
      if (interruptFlagsMask > UINT32_MAX || interruptFlagsMask == 0) {
        throw std::invalid_argument(
            std::string("Invalid interrupt flags mask in record address: ")
                + token);
      }
      this->interruptFlagsMask = interruptFlagsMask;
    } else {
      throw std::invalid_argument(
          std::string("Unrecognized token in record address: ") + token);
    }
    std::tie(tokenStart, tokenLength) = findNextToken(addressString, delimiters,
        tokenStart + tokenLength);
  }
}

} // namespace epics
} // namespace mrf
} // namespace anka
