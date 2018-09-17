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

// The aSubRecord structure has a field called "not". Therefore, we cannot use
// this data structure from C++ code because "not" is a reserved keyword in C++.
#include <stdint.h>
#include <string.h>

#include <aSubRecord.h>
#include <dbFldTypes.h>
// registryFunction.h is needed by epicsExport.h but not included by it.
#include <registryFunction.h>
#include <epicsExport.h>

/**
 * Reads elements from INPA and writes them to OUTA.
 */
static long mrfArrayCopy(aSubRecord *record) {
  // Input and output elements must be of the same type.
  if (record->fta != record->ftva) {
    return 1;
  }
  // The number of input elements must match the number of output elements.
  if (record->noa != record->nova) {
    return 1;
  }
  size_t numberOfElements = record->noa;
  size_t elementSize;
  switch (record->fta) {
  case DBF_CHAR:
  case DBF_UCHAR:
    elementSize = 1;
    break;
  case DBF_SHORT:
  case DBF_USHORT:
    elementSize = 2;
    break;
  case DBF_LONG:
  case DBF_ULONG:
  case DBF_FLOAT:
    elementSize = 4;
    break;
  case DBF_DOUBLE:
    elementSize = 8;
    break;
  default:
    // We do not support other field types.
    return 1;
  }
  memcpy(record->vala, record->a, numberOfElements * elementSize);
  return 0;
}

epicsRegisterFunction(mrfArrayCopy);
