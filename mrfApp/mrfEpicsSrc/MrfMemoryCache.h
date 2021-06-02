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

#ifndef ANKA_MRF_EPICS_MEMORY_CACHE_H
#define ANKA_MRF_EPICS_MEMORY_CACHE_H

#include <cstdint>
#include <mutex>
#include <unordered_map>

#include <MrfMemoryAccess.h>

namespace anka {
namespace mrf {
namespace epics {

/**
 * Read cache for a {@link MrfMemoryAccess}. When a read operation from a memory
 * address is requested for the first time, the cache delegates to the actual
 * memory access. Subsequent read requests for the same address return the
 * cached value instead of delegating to the memory access. This is useful for
 * applications that have to read from the same memory address multiple times,
 * but do not require recent data. Internally, separate caches are used for
 * 16-bit and 32-bit registers.
 *
 * In particular, we use it during record initialization, so that a slow memory
 * access (e.g. one using the UDP/IP protocol) does not slow down the
 * initialization of many records that refer to the same register (e.g. a
 * register that acts as a bit field).
 */
class MrfMemoryCache {

public:

  /**
   * Creates a cache using the specified memory-access. The wrapped memory
   * access must be kept alive until this cache is not used any longer.
   */
  explicit MrfMemoryCache(MrfMemoryAccess &memoryAccess);

  /**
   * Creates a cache wrapping the specified  memory-access. The shared pointer
   * to the memory-access is kept alive until this cache is destroyed.
   */
  explicit MrfMemoryCache(std::shared_ptr<MrfMemoryAccess> memoryAccess);

  /**
   * Reads from an unsigned 16-bit register. The method blocks until the
   * operation has finished (either successfully or unsuccessfully). On success,
   * the value read from the specified memory address is returned. On failure,
   * an exception is thrown. If the value for the specified address is in the
   * cache, the cached value is returned. Otherwise, this method delegates the
   * read operation to the memory access which has been passed to the
   * constructor and caches the read value for subsequent read operations.
   */
  std::uint16_t readUInt16(std::uint32_t address);

  /**
   * Reads from an unsigned 32-bit register. The method blocks until the
   * operation has finished (either successfully or unsuccessfully). On success,
   * the value read from the specified memory address is returned. On failure,
   * an exception is thrown.   If the value for the specified address is in the
   * cache, the cached value is returned. Otherwise, this method delegates the
   * read operation to the memory access which has been passed to the
   * constructor and caches the read value for subsequent read operations.
   */
  std::uint32_t readUInt32(std::uint32_t address);

private:

  // We do not want to allow copy or move construction or assignment.
  MrfMemoryCache(const MrfMemoryCache &) = delete;
  MrfMemoryCache(MrfMemoryCache &&) = delete;
  MrfMemoryCache &operator=(const MrfMemoryCache &) = delete;
  MrfMemoryCache &operator=(MrfMemoryCache &&) = delete;

  MrfMemoryAccess &memoryAccess;
  std::shared_ptr<MrfMemoryAccess> memoryAccessPtr;

  std::recursive_mutex mutex;

  std::unordered_map<std::uint32_t, std::uint16_t> cacheUInt16;
  std::unordered_map<std::uint32_t, std::uint32_t> cacheUInt32;

};

}
}
}

#endif // ANKA_MRF_EPICS_MEMORY_CACHE_H
