/*
 * Copyright 2015 aquenos GmbH.
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

#include <stdexcept>

#include "mrfErrorUtil.h"

#include "MrfTime.h"

namespace anka {
namespace mrf {

MrfTime MrfTime::now() {
  timeval systemTime;
  if (::gettimeofday(&systemTime, nullptr)) {
    throw systemErrorFromErrNo("Could not get system time");
  }
  return MrfTime(systemTime);
}

MrfTime::MrfTime() :
    seconds(0), nanoseconds(0) {
}

MrfTime::MrfTime(std::int_fast64_t seconds, std::int_fast32_t nanoseconds) :
    seconds(seconds), nanoseconds(nanoseconds) {
  if (nanoseconds < 0 || nanoseconds >= 1000000000) {
    throw std::invalid_argument("Invalid nanoseconds value.");
  }
}

MrfTime::MrfTime(const MrfTime &copyFrom) :
    seconds(copyFrom.seconds), nanoseconds(copyFrom.nanoseconds) {
}

MrfTime::MrfTime(const ::timespec &copyFrom) :
    seconds(copyFrom.tv_sec), nanoseconds(copyFrom.tv_nsec) {
  if (copyFrom.tv_nsec < 0 || copyFrom.tv_nsec >= 1000000000) {
    throw std::invalid_argument("Invalid nanoseconds value.");
  }
}

MrfTime::MrfTime(const ::timeval &copyFrom) :
    seconds(copyFrom.tv_sec), nanoseconds(copyFrom.tv_usec * 1000) {
  if (copyFrom.tv_usec < 0 || copyFrom.tv_usec >= 1000000) {
    throw std::invalid_argument("Invalid microseconds value.");
  }
}

MrfTime &MrfTime::operator=(const MrfTime &assignFrom) {
  this->seconds = assignFrom.seconds;
  this->nanoseconds = assignFrom.nanoseconds;
  return *this;
}

MrfTime::operator ::timespec() const {
  ::timespec value;
  value.tv_sec = this->seconds;
  value.tv_nsec = this->nanoseconds;
  return value;
}

MrfTime::operator ::timeval() const {
  ::timeval value;
  value.tv_sec = this->seconds;
  value.tv_usec = this->nanoseconds / 1000;
  return value;
}

std::int_fast64_t MrfTime::getSeconds() const {
  return this->seconds;
}

std::int_fast32_t MrfTime::getNanoseconds() const {
  return this->nanoseconds;
}

bool MrfTime::operator==(const MrfTime &other) const {
  return this->seconds == other.seconds
      && this->nanoseconds == other.nanoseconds;
}

bool MrfTime::operator<(const MrfTime &other) const {
  if (this->seconds == other.seconds) {
    return this->nanoseconds < other.nanoseconds;
  } else {
    return this->seconds < other.seconds;
  }
}

bool MrfTime::operator>(const MrfTime &other) const {
  if (this->seconds == other.seconds) {
    return this->nanoseconds > other.nanoseconds;
  } else {
    return this->seconds > other.seconds;
  }
}

MrfTime &MrfTime::operator+=(const MrfTime &other) {
  this->seconds += other.seconds;
  this->nanoseconds += other.nanoseconds;
  if (this->nanoseconds >= 1000000000) {
    this->nanoseconds -= 1000000000;
    this->seconds += 1;
  }
  return *this;
}

MrfTime &MrfTime::operator-=(const MrfTime &other) {
  this->seconds -= other.seconds;
  this->nanoseconds -= other.nanoseconds;
  if (this->nanoseconds < 0) {
    this->nanoseconds += 1000000000;
    this->seconds -= 1;
  }
  return *this;
}

}
}
