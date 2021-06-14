/*
 * Copyright 2015-2021 aquenos GmbH.
 * Copyright 2015-2021 Karlsruhe Institute of Technology.
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

#ifndef ANKA_MRF_EPICS_RECORD_H
#define ANKA_MRF_EPICS_RECORD_H

#include <memory>
#include <stdexcept>

#include <callback.h>
#include <dbCommon.h>
#include <dbScan.h>

#include <MrfConsistentMemoryAccess.h>

#include "MrfDeviceRegistry.h"
#include "MrfRecordAddress.h"

namespace anka {
namespace mrf {
namespace epics {

/**
 * Base class for most EPICS record device support classes.
 */
template<typename RecordTypeParam>
class MrfRecord {

public:

  /**
   * Type of data structure used by the supported record.
   */
  using RecordType = RecordTypeParam;

  /**
   * Called each time the record is processed. Used for reading (input
   * records) or writing (output records) data from or to the hardware. The
   * default implementation of the processRecord method works asynchronously by
   * calling the {@link #processPrepare()} method and setting the PACT field to
   * one before returning. When it is called again later, PACT is reset to zero
   * and the {@link #processComplete} is called.
   */
  virtual void processRecord();

protected:

  /**
   * Constructor. The record is stored in an attribute of this class and
   * is used by all methods which want to access record fields.
   */
  MrfRecord(RecordType *record, const ::DBLINK &addressField);

  /**
   * Destructor.
   */
  virtual ~MrfRecord() {
  }

  /**
   * Returns the device represented by this record.
   */
  inline std::shared_ptr<MrfConsistentMemoryAccess> getDevice() const {
    return device;
  }

  /**
   * Returns the address associated with this record.
   */
  inline const MrfRecordAddress &getRecordAddress() const {
    return address;
  }

  /**
   * Returns a pointer to the structure that holds the actual EPICS record.
   * Always returns a valid pointer.
   */
  inline RecordType *getRecord() const {
    return record;
  }

  /**
   * Returns a bit mask where only those bits are set that are used for the
   * actual value. This mask is calculated from the highest and lowest bit
   * specified in the record address.
   */
  inline std::uint32_t getMask() const {
    return mask;
  }

  /**
   * Updates the record's value with the specified value.
   */
  virtual void writeRecordValue(std::uint32_t value) = 0;

  /**
   * Converts a value read from a device register to a value that can be used by
   * the record. This is done by applying a mask and shifting the value so that
   * only the bits specified in the device address are used.
   */
  std::uint32_t convertFromDevice(std::uint32_t value);

  /**
   * Converts a value used by the record to a value that can be written to the
   * device register. This is done shifting the value and applying a mask so
   * that only the bits specified in the device address are used. The other bits
   * are set to zero.
   */
  std::uint32_t convertToDevice(std::uint32_t value);

  /**
   * Called by the default implementation of the {@link #processRecord()}
   * method. This method should queue an asynchronous action that calls
   * {@link #scheduleProcessing()} when it finishes.
   */
  virtual void processPrepare() = 0;

  /**
   * Called by the default implementation of the {@link #processRecord()} method.
   * This method is called the second time the record is processed, after the
   * processing has been scheduled using {@link #scheduleProcessing()}. It
   * should update the record with the new value and / or error state.
   */
  virtual void processComplete() = 0;

  /**
   * Schedules processing of the record. This method should only be called from
   * an asynchronous callback that has been scheduled by the
   * {@link processPrepare()} method.
   */
  void scheduleProcessing();

private:

  // We do not want to allow copy or move construction or assignment.
  MrfRecord(const MrfRecord &) = delete;
  MrfRecord(MrfRecord &&) = delete;
  MrfRecord &operator=(const MrfRecord &) = delete;
  MrfRecord &operator=(MrfRecord &&) = delete;

  /**
   * Address specified in the INP our OUT field of the record.
   */
  MrfRecordAddress address;

  /**
   * Pointer to the underlying device.
   */
  std::shared_ptr<MrfConsistentMemoryAccess> device;

  /**
   * Record this device support has been instantiated for.
   */
  RecordType *record;

  /**
   * Callback needed to queue a request for processRecord to be run again.
   */
  ::CALLBACK processCallback;

  /**
   * Bit mask applied to the raw value. Only those bits of the value that are
   * set in the mask are considered relevant.
   */
  std::uint32_t mask;

  /**
   * Reads the record address from an address field.
   */
  static MrfRecordAddress readRecordAddress(const ::DBLINK &addressField);

};

template<typename RecordType>
MrfRecord<RecordType>::MrfRecord(RecordType *record,
    const ::DBLINK &addressField) :
    address(readRecordAddress(addressField)), record(record) {
  this->device = MrfDeviceRegistry::getInstance().getDevice(
      this->address.getDeviceId());
  if (!this->device) {
    throw std::runtime_error(
        std::string("Could not find device ") + this->address.getDeviceId()
            + ".");
  }
  // Initialize the bit mask. By doing this here, we can keep the code run for
  // each read or write operation shorter.
  signed char highestBit = this->address.getMemoryAddressHighestBit();
  signed char lowestBit = this->address.getMemoryAddressLowestBit();
  for (int i = 31; i >= 0; i--) {
    this->mask <<= 1;
    if (i <= highestBit && i >= lowestBit) {
      this->mask |= 1;
    } else {
      this->mask &= ~1;
    }
  }
}

template<typename RecordType>
void MrfRecord<RecordType>::processRecord() {
  if (this->record->pact) {
    this->record->pact = false;
    processComplete();
  } else {
    processPrepare();
    this->record->pact = true;
  }
}

template<typename RecordType>
std::uint32_t MrfRecord<RecordType>::convertFromDevice(std::uint32_t value) {
  value &= mask;
  value >>= getRecordAddress().getMemoryAddressLowestBit();
  return value;
}

template<typename RecordType>
std::uint32_t MrfRecord<RecordType>::convertToDevice(std::uint32_t value) {
  value <<= getRecordAddress().getMemoryAddressLowestBit();
  value &= mask;
  return value;
}

template<typename RecordType>
void MrfRecord<RecordType>::scheduleProcessing() {
  // Registering the callback establishes a happens-before relationship due to
  // an internal lock. Therefore, data written before registering the callback
  // is seen by the callback function.
  ::callbackRequestProcessCallback(&this->processCallback, priorityMedium,
      this->record);
}

template<typename RecordType>
MrfRecordAddress MrfRecord<RecordType>::readRecordAddress(
    const ::DBLINK &addressField) {
  if (addressField.type != INST_IO) {
    throw std::runtime_error(
        "Invalid device address. Maybe mixed up INP/OUT or forgot '@'?");
  }
  return MrfRecordAddress(
      addressField.value.instio.string == nullptr ?
          "" : addressField.value.instio.string);
}

}
}
}

#endif // ANKA_MRF_EPICS_RECORD_H
