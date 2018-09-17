/*
 * Copyright 2015-2016 aquenos GmbH.
 * Copyright 2015-2016 Karlsruhe Institute of Technology.
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

#ifndef ANKA_MRF_ERROR_UTIL_H
#define ANKA_MRF_ERROR_UTIL_H

#include <string>
#include <system_error>

extern "C" {
#include <errno.h>
#include <string.h>
}

namespace anka {
namespace mrf {

/**
 * Returns a textual representation of the specified error code. For generating
 * the text message, the error code is interpreted in the same way as an error
 * code stored in the global {@code errno} variable.
 */
inline std::string errorStringForErrNo(int errorNumber) {
  constexpr int bufferSize = 1024;
  char buffer[bufferSize];
  // The GNU version of strerror_r (which is typically used on Linux) has a
  // slightly different signature than the XSI compliant version.
#if !__linux__ || ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE)
  // Use XSI version.
  int result = ::strerror_r(errorNumber, buffer, bufferSize);
  if (result) {
    return std::string("Unknown error code ") + std::to_string(errno);
  } else {
    return std::string(buffer);
  }
#else
  // Use GNU version.
  return std::string(::strerror_r(errorNumber, buffer, bufferSize));
#endif
}

/**
 * Returns a textual representation of the error code that is stored in the
 * global {@code errno} variable.
 */
inline std::string errorStringFromErrNo() {
  return errorStringForErrNo(errno);
}

/**
 * Creates a system_error that is initialized with the specified error code and
 * a text message that contains both the specified message and a textual
 * representation of the error code. For generating the text message, the error
 * code is interpreted in the same way as an error code stored in the global
 * errno variable.
 */
inline std::system_error systemErrorForErrNo(const std::string &message,
    int errorNumber) {
  return std::system_error(errorNumber, std::system_category(),
      message + ": " + errorStringForErrNo(errorNumber));
}

/**
 * Creates a system_error that is initialized with the global errno and a
 * text message that contains both the specified message and a textual
 * representation of the error code.
 */
inline std::system_error systemErrorFromErrNo(const std::string &message) {
  return systemErrorForErrNo(message, errno);
}

}
}

#endif // ANKA_MRF_ERROR_UTIL_H
