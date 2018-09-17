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

#include <stdexcept>

#include <MrfConsistentAsynchronousMemoryAccess.h>

#include "MrfDeviceRegistry.h"

namespace anka {
namespace mrf {
namespace epics {

std::shared_ptr<MrfConsistentMemoryAccess> MrfDeviceRegistry::getDevice(
    const std::string &deviceId) {
  // We have to hold the mutex in order to protect the map from concurrent
  // access.
  std::lock_guard<std::recursive_mutex> lock(mutex);
  auto device = devices.find(deviceId);
  if (device == devices.end()) {
    return std::shared_ptr<MrfConsistentMemoryAccess>();
  } else {
    return device->second;
  }
}

std::shared_ptr<MrfMemoryCache> MrfDeviceRegistry::getDeviceCache(
    const std::string &deviceId) {
  // We have to hold the mutex in order to protect the map from concurrent
  // access.
  std::lock_guard<std::recursive_mutex> lock(mutex);
  auto cache = caches.find(deviceId);
  if (cache == caches.end()) {
    return std::shared_ptr<MrfMemoryCache>();
  } else {
    return cache->second;
  }
}

void MrfDeviceRegistry::registerDevice(const std::string &deviceId,
    std::shared_ptr<MrfConsistentMemoryAccess> device) {
  // We have to hold the mutex in order to protect the map from concurrent
  // access.
  std::lock_guard<std::recursive_mutex> lock(mutex);
  if (devices.count(deviceId)) {
    throw std::runtime_error("Device ID is already in use.");
  }
  devices.insert(std::make_pair(deviceId, device));
  caches.insert(
      std::make_pair(deviceId, std::make_shared<MrfMemoryCache>(device)));
}

MrfDeviceRegistry MrfDeviceRegistry::instance;

MrfDeviceRegistry::MrfDeviceRegistry() {
}

}
}
}
