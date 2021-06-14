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

#ifndef ANKA_MRF_EPICS_WAVEFORM_OUT_RECORD_H
#define ANKA_MRF_EPICS_WAVEFORM_OUT_RECORD_H

#include <cstdint>
#include <mutex>
#include <vector>

#include <callback.h>
#include <waveformRecord.h>

#include <MrfConsistentMemoryAccess.h>
#include "MrfRecordAddress.h"

namespace anka {
namespace mrf {
namespace epics {

/**
 * Device support class for a waveform record that is used as an output.
 *
 * This device support only supports 32-bit registers. The record's element type
 * must be an integer type (CHAR, UCHAR, SHORT, USHORT, LONG, or ULONG). For
 * data types that are smaller than LONG or ULONG, data received from the device
 * is truncated. The device support writes all elements of the record's value
 * array to the memory of the MRF device, starting at the specified address.
 */
class MrfWaveformOutRecord {

public:

  /**
   * Type of data structure used by the supported record.
   */
  using RecordType = ::waveformRecord;

  /**
   * Creates an instance of the device support for the specified record.
   */
  MrfWaveformOutRecord(::waveformRecord *record);

  /**
   * Called each time the record is processed. Used for writing data to the
   * hardware. This  method works asynchronously by queuing a write request and
   * setting the PACT field to one before returning. When it is called again
   * later, PACT is reset to zero and the processing is completed.
   */
  void processRecord();

private:

  /**
   * Callback implementation used for writing array elements.
   */
  struct CallbackImpl: MrfMemoryAccess::CallbackUInt32 {
    MrfWaveformOutRecord &deviceSupport;
    CallbackImpl(MrfWaveformOutRecord &deviceSupport) :
        deviceSupport(deviceSupport) {
    }
    void success(std::uint32_t address, std::uint32_t value);
    void failure(std::uint32_t address, MrfMemoryAccess::ErrorCode errorCode,
        const std::string &details);
  };

  // We do not want to allow copy or move construction or assignment.
  MrfWaveformOutRecord(const MrfWaveformOutRecord &) = delete;
  MrfWaveformOutRecord(MrfWaveformOutRecord &&) = delete;
  MrfWaveformOutRecord &operator=(const MrfWaveformOutRecord &) = delete;
  MrfWaveformOutRecord &operator=(MrfWaveformOutRecord &&) = delete;

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
  ::waveformRecord *record;

  /**
   * Callback needed to queue a request for processRecord to be run again.
   */
  ::CALLBACK processCallback;

  /**
   * Callback used when writing array elements.
   */
  std::shared_ptr<CallbackImpl> writeCallback;

  /**
   * Flag indicating whether the write operation was successful. This flag is
   * set when the callback is processed and is used by the record processing
   * routine to determine whether the record's alarm state should be set.
   */
  bool writeSuccessful;

  /**
   * If the write operation was not successful, this field stores the respective
   * error message.
   */
  std::string writeErrorMessage;

  /**
   * Number of pending write requests. This is used by the write callback to
   * determine when the last response has been received and the record should
   * be processed again.
   */
  std::uint32_t pendingWriteRequests;

  /**
   * Contains the value written as part of the last write attempt. This is used
   * for checking whether an element has been written successfully and to check
   * whether an element has changed if only changed elements shall be written.
   */
  std::vector<std::uint32_t> lastValueWritten;

  /**
   * Tells whether the corresponding element of lastValueWritten is valid. If
   * the element is not valid, it should always be written, even if its value
   * matches the new value.
   */
  std::vector<bool> lastValueWrittenValid;

};

}
}
}

#endif // ANKA_MRF_EPICS_WAVEFORM_OUT_RECORD_H
