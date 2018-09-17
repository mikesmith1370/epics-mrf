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

#ifndef ANKA_MRF_EPICS_RECORD_ADDRESS_H
#define ANKA_MRF_EPICS_RECORD_ADDRESS_H

#include <cstdint>
#include <string>

namespace anka {
namespace mrf {
namespace epics {

/**
 * Record address for the MRF memory device support.
 */
class MrfRecordAddress {

public:

  /**
   * Type of the memory register.
   */
  enum class DataType {
    /**
     * Unsigned 16-bit register.
     */
    uInt16,

    /**
     * Unsigned 32-bit register.
     */
    uInt32
  };

  /**
   * Creates a record address from a string. Throws an std::invalid_argument
   * exception if the address string does not specify a valid address.
   */
  MrfRecordAddress(const std::string &addressString);

  /**
   * Returns the string identifying the device.
   */
  inline const std::string &getDeviceId() const {
    return deviceId;
  }

  /**
   * Returns the distance between two elements (in bytes). This is only used for
   * arrays and (if non-zero) means, that there are gaps between the memory
   * addresses of array elements. This is useful for memory areas that represent
   * tables where the "columns" are interleaved. The default is zero, meaning
   * that the array is represented by a contiguous area in memory.
   */
  inline int getElementDistance() const {
    return elementDistance;
  }

  /**
   * Returns the address of the memory register that shall be read or written.
   */
  inline std::uint32_t getMemoryAddress() const {
    return address;
  }

  /**
   * Returns the index of the highest bit of the memory register that shall be
   * used. The index is zero based and the returned value is always in the
   * interval [0, register width). It is also greater than or equal to the
   * index returned by {@link #getMemoryAddressLowestBit}.
   */
  inline signed char getMemoryAddressHighestBit() const {
    return highestBit;
  }

  /**
   * Returns the index of the lowest bit of the memory register that shall be
   * used. The index is zero based and the returned value is always in the
   * interval [0, register width). It is also less than or equal to the
   * index returned by {@link #getMemoryAddressHighestBit}.
   */
  inline signed char getMemoryAddressLowestBit() const {
    return lowestBit;
  }

  /**
   * Returns the data type of the register. The data type also implies the
   * register width.
   */
  inline DataType getDataType() const {
    return dataType;
  }

  /**
   * Tells whether the bits that are not used should be forced to zero when
   * writing to the register. This is useful for registers that have bits that
   * are not used or which do not represent a state but just trigger an event
   * when set to one. In this case, writing zero to the unused bits can save the
   * read operation which is usually required to determine the value of the bits
   * that are not used by the record.
   */
  inline bool isZeroOtherBits() const {
    return zeroOtherBits;
  }

  /**
   * Tells whether the bits written should be verified. If <code>true</code>,
   * the value that is sent by the device after writing is compared with the
   * value written. If it does not match, the write is considered not
   * successful. If <code>false</code>, the value sent by the device is
   * discarded. This is useful for registers that do not return the value that
   * has been written to them (e.g. registers that acts as a trigger). If this
   * flag is <code>false</code>, this implies that the read-on-init flag is also
   * <code>false</code>. For input records, this flag does not have any effects.
   */
  inline bool isVerify() const {
    return verify;
  }

  /**
   * Tells whether the record should be initialized with the value read from the
   * device. If <code>true</code>, the current value is read once during record
   * initialization. If <code>false</code>, the value is never read. This flag
   * is always <code>false</code> if the verify flag is <code>false</code>. For
   * input records, this flag does not have any effects.
   */
  inline bool isReadOnInit() const {
    return readOnInit;
  }

  /**
   * Tells whether a write operation should send all elements of an array to the
   * device or just the one that have been changed. This flag only has an
   * effect for array values. If <code>true</code> only those elements of the
   * value that have been changed since the last write operation are sent to the
   * device. If <code>false</code> the complete array is sent to the device.
   * This flag is useful with large arrays where sending all data to the device
   * can take a considerable amount of time.
   */
  inline bool isChangedElementsOnly() const {
    return changedElementsOnly;
  }

private:

  std::string deviceId;

  std::uint32_t address;
  signed char highestBit;
  signed char lowestBit;

  DataType dataType;

  int elementDistance;
  bool zeroOtherBits;
  bool verify;
  bool readOnInit;
  bool changedElementsOnly;

};

}
}
}

#endif // ANKA_MRF_EPICS_RECORD_ADDRESS_H
