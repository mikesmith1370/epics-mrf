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

#include "MrfMemoryCache.h"

namespace anka {
namespace mrf {
namespace epics {

MrfMemoryCache::MrfMemoryCache(MrfMemoryAccess &memoryAccess) :
    memoryAccess(memoryAccess) {
}

MrfMemoryCache::MrfMemoryCache(std::shared_ptr<MrfMemoryAccess> memoryAccess) :
    memoryAccess(*memoryAccess), memoryAccessPtr(memoryAccess) {
}

std::map<std::uint32_t, std::uint16_t> MrfMemoryCache::getCacheUInt16() const {
  // Access to the hash map has to be protected by a mutex.
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return std::map<std::uint32_t, std::uint16_t>(
    cacheUInt16.begin(), cacheUInt16.end());
}

std::map<std::uint32_t, std::uint32_t> MrfMemoryCache::getCacheUInt32() const {
  // Access to the hash map has to be protected by a mutex.
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return std::map<std::uint32_t, std::uint32_t>(
    cacheUInt32.begin(), cacheUInt32.end());
}

std::uint16_t MrfMemoryCache::readUInt16(std::uint32_t address) {
  {
    // Access to the hash map has to be protected by a mutex.
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto iterator = cacheUInt16.find(address);
    if (iterator != cacheUInt16.end()) {
      return iterator->second;
    }
  }
  // If the register is not in the cache yet, we read it from the memory access.
  // We do this without holding the mutex because the read operation might take
  // a lot of time and we do not want to block other read operations in the
  // meantime. This means that the same register might be read more than once,
  // if multiple read attempt are made before the first one is finished.
  // However, this is still better than the alternative.
  std::uint16_t value = memoryAccess.readUInt16(address);
  {
    // Access to the hash map has to be protected by a mutex.
    std::lock_guard<std::recursive_mutex> lock(mutex);
    // If the value has already been cached, we prefer the cached value. This
    // way, the behavior is independent concurrency timings.
    return cacheUInt16.emplace(address, value).first->second;
  }
}

std::uint32_t MrfMemoryCache::readUInt32(std::uint32_t address) {
  {
    // Access to the hash map has to be protected by a mutex.
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto iterator = cacheUInt32.find(address);
    if (iterator != cacheUInt32.end()) {
      return iterator->second;
    }
  }
  // If the register is not in the cache yet, we read it from the memory access.
  // We do this without holding the mutex because the read operation might take
  // a lot of time and we do not want to block other read operations in the
  // meantime. This means that the same register might be read more than once,
  // if multiple read attempt are made before the first one is finished.
  // However, this is still better than the alternative.
  std::uint32_t value = memoryAccess.readUInt32(address);
  {
    // Access to the hash map has to be protected by a mutex.
    std::lock_guard<std::recursive_mutex> lock(mutex);
    // If the value has already been cached, we prefer the cached value. This
    // way, the behavior is independent concurrency timings.
    return cacheUInt32.emplace(address, value).first->second;
  }
}

void MrfMemoryCache::tryCacheUInt16(std::uint32_t address) {
  try {
    readUInt16(address);
  } catch (...) {
    // We ignore any exception.
  }
}

void MrfMemoryCache::tryCacheUInt32(std::uint32_t address) {
  try {
    readUInt32(address);
  } catch (...) {
    // We ignore any exception.
  }
}

}
}
}
