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

#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include <epicsExport.h>
#include <iocsh.h>

#include <MrfConsistentAsynchronousMemoryAccess.h>
#include <MrfDeviceRegistry.h>
#include <MrfTime.h>
#include <MrfUdpIpMemoryAccess.h>
#include <mrfEpicsError.h>

using namespace anka::mrf;
using namespace anka::mrf::epics;

/**
 * Creates (and registers) a UDP/IP device. EVG and EVR devices are nearly
 * identical with the exception that they use a different base address.
 */
static void createUdpIpDevice(const std::string& deviceId,
    const std::string &hostName, std::uint32_t baseAddress,
    const MrfTime &delayBetweenPackets, const MrfTime &udpTimeout,
    int maximumNumberOfTries) {
  std::shared_ptr<MrfUdpIpMemoryAccess> rawDevice = std::make_shared<
      MrfUdpIpMemoryAccess>(hostName, baseAddress, delayBetweenPackets,
      udpTimeout, maximumNumberOfTries);
  std::shared_ptr<MrfConsistentAsynchronousMemoryAccess> consistentDevice =
      std::make_shared<MrfConsistentAsynchronousMemoryAccess>(rawDevice);
  MrfDeviceRegistry::getInstance().registerDevice(std::string(deviceId),
      consistentDevice);
}

/**
 * Creates an EVG device with the specified ID that is accessed using the
 * UDP/IP based protocol. Throws an exception if the device cannot be created
 * (e.g. because the device ID is already in use).
 */
static void createUdpIpEvgDevice(const std::string& deviceId,
    const std::string &hostName, const MrfTime &delayBetweenPackets,
    const MrfTime &udpTimeout, int maximumNumberOfTries) {
  createUdpIpDevice(deviceId, hostName,
      MrfUdpIpMemoryAccess::baseAddressVmeEvgRegister, delayBetweenPackets,
      udpTimeout, maximumNumberOfTries);
}

/**
 * Creates an EVR device with the specified ID that is accessed using the
 * UDP/IP based protocol. Throws an exception if the device cannot be created
 * (e.g. because the device ID is already in use).
 */
void createUdpIpEvrDevice(const std::string& deviceId,
    const std::string &hostName, const MrfTime &delayBetweenPackets,
    const MrfTime &udpTimeout, int maximumNumberOfTries) {
  createUdpIpDevice(deviceId, hostName,
      MrfUdpIpMemoryAccess::baseAddressVmeEvrRegister, delayBetweenPackets,
      udpTimeout, maximumNumberOfTries);
}

extern "C" {

// Data structures needed for the iocsh mrfUdpIpDevice function.
static const iocshArg iocshMrfUdpIpDeviceArg0 = { "device ID", iocshArgString };
static const iocshArg iocshMrfUdpIpDeviceArg1 = { "host name or address",
    iocshArgString };
static const iocshArg iocshMrfUdpIpDeviceArg2 = {
    "min. delay between consecutive UDP packets (seconds)", iocshArgDouble };
static const iocshArg iocshMrfUdpIpDeviceArg3 = { "UDP timeout (seconds)",
    iocshArgDouble };
static const iocshArg iocshMrfUdpIpDeviceArg4 = { "max. number of tries",
    iocshArgInt };
static const iocshArg * const iocshMrfUdpIpDeviceArgs[] =
    { &iocshMrfUdpIpDeviceArg0, &iocshMrfUdpIpDeviceArg1,
        &iocshMrfUdpIpDeviceArg2, &iocshMrfUdpIpDeviceArg3,
        &iocshMrfUdpIpDeviceArg4 };
static const iocshFuncDef iocshMrfUdpIpEvgDeviceFuncDef = { "mrfUdpIpEvgDevice",
    5, iocshMrfUdpIpDeviceArgs };
static const iocshFuncDef iocshMrfUdpIpEvrDeviceFuncDef = { "mrfUdpIpEvrDevice",
    5, iocshMrfUdpIpDeviceArgs };

/**
 * Common implementation of the iocsh mrfUdpIpEvgDevice and mrfUdpIpEvrDevice
 * functions.
 */
static void iocshMrfUdpIpDeviceFunc(const iocshArgBuf *args, bool evr)
    noexcept {
  char *deviceId = args[0].sval;
  char *hostAddress = args[1].sval;
  double delayBetweenPacketsDouble = args[2].dval;
  double udpTimeoutDouble = args[3].dval;
  int maxNumberOfTries = args[4].ival;
  // Verify and convert the parameters.
  if (!deviceId) {
    errorPrintf("Could not create device: Device ID must be specified.");
    return;
  }
  if (!std::strlen(deviceId)) {
    errorPrintf("Could not create device: Device ID must not be empty.");
    return;
  }
  // Until here our code does not throw. We put the rest of the function into a
  // try-catch statement, so that we handle all other exceptions.
  try {
    if (!hostAddress) {
      throw std::invalid_argument("Host name or address must be specified.");
    }
    if (!std::strlen(hostAddress)) {
      throw std::invalid_argument("Host name or address must not be empty.");
    }
    if (!std::isfinite(delayBetweenPacketsDouble)) {
      throw std::invalid_argument(
          "Min. delay between consecutive UDP packets must be a finite value.");
    }
    if (delayBetweenPacketsDouble <= 0.0) {
      // We use a default value of 400 us. Experiments have shown that this is
      // a good value.
      delayBetweenPacketsDouble = 0.0004;
    }
    // We have to set an upper limit on the delay because it has to be converted
    // to an integer. We could allow a larger value, but such a value would not
    // make sense anyway.
    if (delayBetweenPacketsDouble > 3600.0) {
      throw std::invalid_argument(
          "Min. delay between consecutive UDP packets must not be greater than 3600 seconds.");
    }
    if (!std::isfinite(udpTimeoutDouble)) {
      throw std::invalid_argument("UDP timeout must be a finite value.");
    }
    if (udpTimeoutDouble <= 0.0) {
      // We use a default value of 5ms. For a direct connection, the typical
      // round-trip time is about 750 us. With a switch involved, it is about
      // 770 us. Therefore, 5 ms should be enough even if there are multiple
      // switches involved.
      udpTimeoutDouble = 0.005;
    }
    // We have to set an upper limit on the timeout because it has to be
    // converted to an integer. We could allow a larger value, but such a value
    // would not make sense anyway.
    if (udpTimeoutDouble > 3600.0) {
      throw std::invalid_argument(
          "UDP timeout must not be greater than 3600 seconds.");
    }
    if (maxNumberOfTries <= 0) {
      // Five retries are a good default value because it should cover most
      // situations where a packet is lost because of network congestion.
      maxNumberOfTries = 5;
    }
    MrfTime delayBetweenPackets(std::floor(delayBetweenPacketsDouble),
        std::remainder(delayBetweenPacketsDouble * 1000000000.0, 1000000000.0));
    MrfTime udpTimeout(std::floor(udpTimeoutDouble),
        std::remainder(udpTimeoutDouble * 1000000000.0, 1000000000.0));
    if (evr) {
      createUdpIpEvrDevice(deviceId, hostAddress, delayBetweenPackets,
          udpTimeout, maxNumberOfTries);
    } else {
      createUdpIpEvgDevice(deviceId, hostAddress, delayBetweenPackets,
          udpTimeout, maxNumberOfTries);
    }
  } catch (std::exception &e) {
    anka::mrf::epics::errorPrintf("Could not create device %s: %s", deviceId,
        e.what());
  } catch (...) {
    anka::mrf::epics::errorPrintf("Could not create device %s: Unknown error.",
        deviceId);
  }
}

/**
 * Implementation of the iocsh mrfUdpIpEvgDevice function.
 */
static void iocshMrfUdpIpEvgDeviceFunc(const iocshArgBuf *args) noexcept {
  iocshMrfUdpIpDeviceFunc(args, false);
}

/**
 * Implementation of the iocsh mrfUdpIpEvrDevice function.
 */
static void iocshMrfUdpIpEvrDeviceFunc(const iocshArgBuf *args) noexcept {
  iocshMrfUdpIpDeviceFunc(args, true);
}

/*
 * Registrar that registers the iocsh commands.
 */
static void mrfRegistrarUdpIp() {
  iocshRegister(&iocshMrfUdpIpEvgDeviceFuncDef, iocshMrfUdpIpEvgDeviceFunc);
  iocshRegister(&iocshMrfUdpIpEvrDeviceFuncDef, iocshMrfUdpIpEvrDeviceFunc);
}

epicsExportRegistrar(mrfRegistrarUdpIp);

}
