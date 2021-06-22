/*
 * Copyright 2021 aquenos GmbH.
 * Copyright 2021 Karlsruhe Institute of Technology.
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

#ifndef ANKA_MRF_EPICS_STRINGIN_RECORD_H
#define ANKA_MRF_EPICS_STRINGIN_RECORD_H

#include <cstdint>
#include <mutex>
#include <vector>

#include <callback.h>
#include <stringinRecord.h>

#include <MrfConsistentMemoryAccess.h>
#include "MrfRecordAddress.h"

namespace anka {
namespace mrf {
namespace epics {

/**
 * Device support class for the stringin record.
 *
 * This device support reads a string from consecutive 16- or 32-bit registers.
 * This means that the string length must be a multiple of two or four bytes
 * respectively.
 */
class MrfStringinRecord {

public:

  /**
   * Type of data structure used by the supported record.
   */
  using RecordType = ::stringinRecord;

  /**
   * Creates an instance of the device support for the specified record.
   */
  MrfStringinRecord(::stringinRecord *record);

  /**
   * Called each time the record is processed. Used for writing data to the
   * hardware. This  method works asynchronously by queuing a write request and
   * setting the PACT field to one before returning. When it is called again
   * later, PACT is reset to zero and the processing is completed.
   */
  void processRecord();

private:

  /**
   * Callback implementation used for reading array elements.
   */
  struct CallbackImpl
      : MrfMemoryAccess::CallbackUInt16, MrfMemoryAccess::CallbackUInt32 {
    MrfStringinRecord &deviceSupport;
    CallbackImpl(MrfStringinRecord &deviceSupport) :
        deviceSupport(deviceSupport) {
    }
    void success(std::uint32_t address, std::uint16_t value);
    void success(std::uint32_t address, std::uint32_t value);
    void failure(std::uint32_t address, MrfMemoryAccess::ErrorCode errorCode,
        const std::string &details);
  };

  // We do not want to allow copy or move construction or assignment.
  MrfStringinRecord(const MrfStringinRecord &) = delete;
  MrfStringinRecord(MrfStringinRecord &&) = delete;
  MrfStringinRecord &operator=(const MrfStringinRecord &) = delete;
  MrfStringinRecord &operator=(MrfStringinRecord &&) = delete;

  /**
   * Maximum length of a string that can be stored in the record (including the
   * terminating null byte).
   */
  const static int maxStringLength = sizeof(::stringinRecord::val);

  /**
   * Mutex that must be hold when processing the record or callbacks. The mutex
   * has to be recursive because callbacks might be triggered from within the
   * process method.
   */
  std::recursive_mutex mutex;

  /**
   * Address specified in the INP field of the record.
   */
  MrfRecordAddress address;

  /**
   * Pointer to the underlying device.
   */
  std::shared_ptr<MrfConsistentMemoryAccess> device;

  /**
   * Record this device support has been instantiated for.
   */
  ::stringinRecord *record;

  /**
   * Callback needed to queue a request for processRecord to be run again.
   */
  ::CALLBACK processCallback;

  /**
   * Callback used when reading array elements.
   */
  std::shared_ptr<CallbackImpl> readCallback;

  /**
   * Flag indicating whether the read operation was successful. This flag is
   * set when the callback is processed and is used by the record processing
   * routine to determine whether the record's alarm state should be set.
   */
  bool readSuccessful;

  /**
   * If the read operation was not successful, this field stores the respective
   * error message.
   */
  std::string readErrorMessage;

  /**
   * Number of pending read requests. This is used by the read callback to
   * determine when the last response has been received and the record should
   * be processed again.
   */
  std::uint32_t pendingReadRequests;

  /**
   * Contains the value read as part of the last read attempt. When the value
   * has been written completely, it is transferred into the records value
   * field.
   */
  char lastValueRead[maxStringLength];

};

}
}
}

#endif // ANKA_MRF_EPICS_STRINGIN_RECORD_H
