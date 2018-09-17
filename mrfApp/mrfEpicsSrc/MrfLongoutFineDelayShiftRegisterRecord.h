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

#ifndef ANKA_MRF_EPICS_LONGOUT_FINE_DELAY_SHIFT_REGISTER_RECORD_H
#define ANKA_MRF_EPICS_LONGOUT_FINE_DELAY_SHIFT_REGISTER_RECORD_H

#include <cstdint>

#include <callback.h>
#include <longoutRecord.h>

#include <MrfConsistentMemoryAccess.h>

namespace anka {
namespace mrf {
namespace epics {

/**
 * Device support class for a longout record that handles the shift register
 * used to control the fine delay of certain universal output modules. On of
 * these records is always responsible for setting the fine delay of two outputs
 * (which both use the same universal output module).
 *
 * When the record is processed, a series of write operations is started, which
 * use four GPIO outputs to configure the two delay chips on a universal output
 * module through their common shift register.
 *
 * The value stored in the record is interpreted in the following way: Bits
 * 0-9 store the delay for the first output and bits 16-25 store the delay for
 * the second output. Bit 10 stores the latch enable bit for the first output
 * and bit 26 stores the latch enable bit for the second output. Typically,
 * these bits should both be zero. If they are not zero, updating the
 * corresponding delay value does not have any effect. Bit 31 is used for the
 * output disable signal. If this bit is set, both outputs are tri-stated,
 * effectively disabling them. The least significant bit is bit 0.
 */
class MrfLongoutFineDelayShiftRegisterRecord {

public:

  /**
   * Creates an instance of the device support for the specified record.
   */
  MrfLongoutFineDelayShiftRegisterRecord(::longoutRecord *record);

  /**
   * Called each time the record is processed. Used for writing data to the
   * hardware. This  method works asynchronously by queuing a write request and
   * setting the PACT field to one before returning. When it is called again
   * later, PACT is reset to zero and the processing is completed.
   */
  void processRecord();

private:

  /**
   * Callback implementation used when writing to the GPIO registers.
   */
  struct CallbackImpl: MrfMemoryAccess::CallbackUInt32 {
    MrfLongoutFineDelayShiftRegisterRecord &deviceSupport;
    CallbackImpl(MrfLongoutFineDelayShiftRegisterRecord &deviceSupport) :
        deviceSupport(deviceSupport) {
    }
    void success(std::uint32_t address, std::uint32_t value);
    void failure(std::uint32_t address, MrfMemoryAccess::ErrorCode errorCode,
        const std::string &details);
  };

  // We do not want to allow copy or move construction or assignment.
  MrfLongoutFineDelayShiftRegisterRecord(
      const MrfLongoutFineDelayShiftRegisterRecord &) = delete;
  MrfLongoutFineDelayShiftRegisterRecord(
      MrfLongoutFineDelayShiftRegisterRecord &&) = delete;
  MrfLongoutFineDelayShiftRegisterRecord &operator=(
      const MrfLongoutFineDelayShiftRegisterRecord &) = delete;
  MrfLongoutFineDelayShiftRegisterRecord &operator=(
      MrfLongoutFineDelayShiftRegisterRecord &&) = delete;

  /**
   * Pointer to the underlying device.
   */
  std::shared_ptr<MrfConsistentMemoryAccess> device;

  /**
   * Record this device support has been instantiated for.
   */
  ::longoutRecord *record;

  /**
   * Callback needed to queue a request for processRecord to be run again.
   */
  ::CALLBACK processCallback;

  /**
   * Callback needed to delay subsequent write requests.
   */
  ::CALLBACK nextTransferStepCallback;

  /**
   * Address of the register used for setting the GPIO direction (relative to
   * the base address).
   */
  std::uint32_t gpioDirectionRegisterAddress;

  /**
   * Index of the lowest bit used in the GPIO direction register. The device
   * support sets this and the three subsequent (more significant bits) to one
   * (output mode).
   */
  signed char gpioDirectionRegisterBitShift;

  /**
   * Address of the register used for setting the GPIO output states (relative
   * to the base address).
   */
  std::uint32_t gpioOutputRegisterAddress;

  /**
   * Index of the lowest bit used in the GPIO output register. The device
   * support uses this bit for the serial data signal (DIN) to the shift
   * register. The next more significant bit is used for the serial clock signal
   * (SCLK), the next, next more significant bit is used for the  transfer latch
   * clock (LCLK), and the next, next, next more significant bit is used for the
   * output disable signal (DIS) to the delay chip.
   */
  signed char gpioOutputRegisterBitShift;

  /**
   * Value to be written. This is the value of the underlying record which is
   * copied when the record processing is started.
   */
  std::uint32_t writeOutputValue;

  /**
   * Value that has been written in the last write operation. This is used to
   * check whether the value has been written correctly when the callback is
   * processed.
   */
  std::uint32_t lastValueWritten;

  /**
   * Mask of the value that has been written in the last write operation. This
   * is used to check whether the value has been written correctly when the
   * callback is processed.
   */
  std::uint32_t lastValueWrittenMask;

  /**
   * Index of the next step in the transfer process. This index start at zero
   * and keeps track of which action has to be taken next.
   */
  int nextTransferStepIndex;

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
   * Callback used for all write operations.
   */
  std::shared_ptr<CallbackImpl> writeCallback;

  /**
   * Schedules the record to be processed again.
   */
  void scheduleProcessing();

  /**
   * Starts the next step of the transfer process. This also increments the
   * next transfer-step index.
   */
  void startNextTransferStep();

  /**
   * Static version of the {@link #startNextTransferStep()} method. This
   * function extracts the instance of this class from the callback and then
   * calls {@link #startNextTransferStep()}.
   */
  static void startNextTransferStepStatic(::CALLBACK *callback);

};

}
}
}

#endif // ANKA_MRF_EPICS_LONGOUT_FINE_DELAY_SHIFT_REGISTER_RECORD_H
