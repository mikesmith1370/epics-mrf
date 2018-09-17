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

#ifndef ANKA_MRF_CONSISTENT_MEMORY_ACCESS_H
#define ANKA_MRF_CONSISTENT_MEMORY_ACCESS_H

#include "MrfMemoryAccess.h"

namespace anka {
namespace mrf {

/**
 * Interface for a memory access that allows to perform updates (read - modify
 * - write operations) in a consistent way.
 */
class MrfConsistentMemoryAccess: public MrfMemoryAccess {

public:

  /**
   * Interface for an updating callback. An updating callback can read and then
   * write a value in a consistent way, without another write or update
   * operation interfering with the update. The callbacks's update method is
   * called after the register has been read. The return value of the update
   * method is then written to the register. Once the update value has been
   * successfully written to the register, the callback's success method is
   * called. If the read or write operation fails, the callback's failure method
   * is called.
   */
  template<typename T>
  class UpdatingCallback: public Callback<T> {
  public:
    /**
     * Called after the value has been read from the register. The old value is
     * passed as a parameter and the method must return the new value which will
     * be written to the register.
     */
    virtual T update(std::uint32_t address, T oldValue) = 0;

    /**
     * Default constructor.
     */
    UpdatingCallback() {
    }

    /**
     * Destructor. Virtual classes should have a virtual destructor.
     */
    virtual ~UpdatingCallback() {
    }

  private:
    // We do not want to allow copy or move construction or assignment.
    UpdatingCallback(const UpdatingCallback &) = delete;
    UpdatingCallback(UpdatingCallback &&) = delete;
    UpdatingCallback &operator=(const UpdatingCallback &) = delete;
    UpdatingCallback &operator=(UpdatingCallback &&) = delete;

  };

  /**
   * Default constructor.
   */
  MrfConsistentMemoryAccess() {
  }

  /**
   * Updating callback for an unsigned 16-bit register.
   */
  using UpdatingCallbackUInt16 = UpdatingCallback<std::uint16_t>;

  /**
   * Updating callback for an unsigned 32-bit register.
   */
  using UpdatingCallbackUInt32 = UpdatingCallback<std::uint32_t>;

  /**
   * Updates an unsigned 16-bit register in a consistent way. The register's
   * value is read, then the callback's update method is called, and finally
   * the updated value is written to the register. Other write or update
   * operations to the same register are blocked until this process has
   * finished. This method does not block.
   */
  virtual void updateUInt16(std::uint32_t address,
      std::shared_ptr<UpdatingCallbackUInt16> callback) =0;

  /**
   * Updates an unsigned 32-bit register in a consistent way. The register's
   * value is read, then the callback's update method is called, and finally
   * the updated value is written to the register. Other write or update
   * operations to the same register are blocked until this process has
   * finished. This method does not block.
   */
  virtual void updateUInt32(std::uint32_t address,
      std::shared_ptr<UpdatingCallbackUInt32> callback) =0;

  /**
   * Writes a value to an unsigned 16-bit register using the specified mask.
   * Only those bits of the value that are set in mask are written to the
   * register. The other bits are copied from the previous value of the
   * register. The method blocks until the operation has finished (either
   * successfully or unsuccessfully). On success, the value read from the memory
   * (after writing to it) is returned. On failure, an exception is thrown.
   */
  virtual std::uint16_t writeUInt16(std::uint32_t address, std::uint16_t value,
      std::uint16_t mask);

  /**
   * Writes a value to an unsigned 16-bit register using the specified mask.
   * Only those bits of the value that are set in mask are written to the
   * register. The other bits are copied from the previous value of the
   * register. This method does not block. The operation is queued and executed
   * asynchronously. When the operation finishes, the specified callback is
   * called. Implementations that can update registers atomically without
   * blocking might call the callback directly in the calling thread.
   */
  virtual void writeUInt16(std::uint32_t address, std::uint16_t value,
      std::uint16_t mask, std::shared_ptr<CallbackUInt16> callback);

  /**
   * Writes a value to an unsigned 32-bit register using the specified mask.
   * Only those bits of the value that are set in mask are written to the
   * register. The other bits are copied from the previous value of the
   * register. The method blocks until the operation has finished (either
   * successfully or unsuccessfully). On success, the value read from the memory
   * (after writing to it) is returned. On failure, an exception is thrown.
   */
  virtual std::uint32_t writeUInt32(std::uint32_t address, std::uint32_t value,
      std::uint32_t mask);

  /**
   * Writes a value to an unsigned 32-bit register using the specified mask.
   * Only those bits of the value that are set in mask are written to the
   * register. The other bits are copied from the previous value of the
   * register. This method does not block. The operation is queued and executed
   * asynchronously. When the operation finishes, the specified callback is
   * called. Implementations that can update registers atomically without
   * blocking might call the callback directly in the calling thread.
   */
  virtual void writeUInt32(std::uint32_t address, std::uint32_t value,
      std::uint32_t mask, std::shared_ptr<CallbackUInt32> callback);

  // We want the methods from the base class to participate in overload
  // resolution.
  using MrfMemoryAccess::writeUInt16;
  using MrfMemoryAccess::writeUInt32;

protected:

  /**
   * The destructor is protected so that an instance cannot be destroyed through
   * a pointer to the base class. Virtual classes should always have a virtual
   * destructor.
   */
  virtual ~MrfConsistentMemoryAccess() {
  }

private:

  // We do not want to allow copy or move construction or assignment.
  MrfConsistentMemoryAccess(const MrfConsistentMemoryAccess &) = delete;
  MrfConsistentMemoryAccess(MrfConsistentMemoryAccess &&) = delete;
  MrfConsistentMemoryAccess &operator=(const MrfConsistentMemoryAccess &) = delete;
  MrfConsistentMemoryAccess &operator=(MrfConsistentMemoryAccess &&) = delete;

};

}
}

#endif // ANKA_MRF_CONSISTENT_MEMORY_ACCESS_H
