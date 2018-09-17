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

#ifndef ANKA_MRF_TIME_H
#define ANKA_MRF_TIME_H

#include <cstdint>

extern "C" {
#include <sys/time.h>
#include <time.h>
}

namespace anka {
namespace mrf {

/**
 * Time value that can represent a point in time or a time difference.
 * Internally the time difference or point in time is represented by two fields,
 * one holding the number of seconds and the other one holding the number of
 * nanoseconds. For an instance representing a point in time, the time is the
 * time that has passed since UNIX epoch (January 1st, 1970, 00:00:00 UTC). The
 * nanoseconds field is always greater than or equal to zero and less than one
 * billion.
 */
class MrfTime {

public:

  /**
   * Creates a time stamp representing the current system time. Throws an
   * exception if the current system time cannot be determined.
   */
  static MrfTime now();

  /**
   * Creates a time stamp with both the seconds and the nanoseconds field
   * initialized to zero.
   */
  MrfTime();

  /**
   * Creates a time value representing the specified number of seconds and
   * nanoseconds. Throws an exception if the specified number of nanoseconds is
   * negative or greater than or equal to one billion.
   */
  MrfTime(std::int_fast64_t seconds, std::int_fast32_t nanoseconds);

  /**
   * Creates a copy of the specified time value.
   */
  MrfTime(const MrfTime &copyFrom);

  /**
   * Converts from a timespec. Throws an exception if the value of the
   * timespec's tv_nsec field is negative or greater than or equal to one
   * billion. If the value of the timespec's tv_sec field is outside the range
   * of the std::int_fast64_t type, the behavior is unspecified.
   */
  MrfTime(const ::timespec &copyFrom);

  /**
   * Converts from a timeval. Throws an exception if the value of the tiemval's
   * tv_usec field is negative or greater than or equal to one million. If the
   * value of the timeval's tv_sec field is outside the range of the
   * std::int_fast32_t type, the behavior is unspecified.
   */
  MrfTime(const ::timeval &copyFrom);

  /**
   * Updates the fields of this time value with the values of the fields of the
   * specified time value.
   */
  MrfTime &operator=(const MrfTime &assignFrom);

  /**
   * Converts this value to a timespec. If the value of this time value's
   * seconds field is outside the range of the type of the timespec's tv_sec
   * field, the behavior is unspecified.
   */
  operator ::timespec() const;

  /**
   * Converts this value to a timeval. If the value of this time value's seconds
   * field is outside the range of the type of the timeval's tv_sec field, the
   * behavior is unspecified. The value of the timeval's tv_usec field is set to
   * the value of this time value's nanoseconds field divided by one thousand
   * without rounding. This means that the resulting microseconds value
   * multiplied by one thousand is always less than or equal to the nanoseconds
   * value.
   */
  operator ::timeval() const;

  /**
   * Returns the value of the seconds field.
   */
  std::int_fast64_t getSeconds() const;

  /**
   * Returns the value of the nanoseconds field. The value is always greater
   * than or equal to zero and less than one billion.
   */
  std::int_fast32_t getNanoseconds() const;

  bool operator==(const MrfTime &other) const;
  bool operator!=(const MrfTime &other) const {
    return !(*this == other);
  }
  bool operator<(const MrfTime &other) const;
  bool operator>(const MrfTime &other) const;
  bool operator<=(const MrfTime &other) const {
    return other > *this;
  }
  bool operator>=(const MrfTime &other) const {
    return other < *this;
  }
  MrfTime &operator+=(const MrfTime &other);
  MrfTime &operator-=(const MrfTime &other);
  MrfTime operator+(const MrfTime &other) const {
    MrfTime sum(*this);
    sum += other;
    return sum;
  }
  MrfTime operator-(const MrfTime &other) const {
    MrfTime difference(*this);
    difference -= other;
    return difference;
  }

private:
  std::int_fast64_t seconds;
  std::int_fast32_t nanoseconds;

};

}
}

#endif // ANKA_MRF_TIME_H
