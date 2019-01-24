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

#include <dbCommon.h>
#include <devSup.h>
#include <epicsExport.h>

#include "MrfAiRecord.h"
#include "MrfAoRecord.h"
#include "MrfBiRecord.h"
#include "MrfBiInterruptRecord.h"
#include "MrfBoRecord.h"
#include "MrfLonginRecord.h"
#include "MrfLonginInterruptRecord.h"
#include "MrfLongoutRecord.h"
#include "MrfLongoutFineDelayShiftRegisterRecord.h"
#include "MrfMbbiDirectRecord.h"
#include "MrfMbbiDirectInterruptRecord.h"
#include "MrfMbboDirectRecord.h"
#include "MrfMbbiRecord.h"
#include "MrfMbboRecord.h"
#include "MrfWaveformInRecord.h"
#include "MrfWaveformOutRecord.h"
#include "mrfEpicsError.h"

using namespace anka::mrf;
using namespace anka::mrf::epics;

namespace {

template<typename RecordDeviceSupportType, typename RecordType>
long initRecord(void *recordVoid) {
  if (!recordVoid) {
    errorExtendedPrintf(
        "Record initialization failed: Pointer to record structure is null.");
    return -1;
  }
  dbCommon *record = static_cast<dbCommon *>(recordVoid);
  try {
    RecordDeviceSupportType *deviceSupport = new RecordDeviceSupportType(
        static_cast<RecordType *>(recordVoid));
    record->dpvt = deviceSupport;
    return 0;
  } catch (std::exception &e) {
    record->dpvt = nullptr;
    errorExtendedPrintf("%s Record initialization failed: %s", record->name,
        e.what());
    return -1;
  } catch (...) {
    record->dpvt = nullptr;
    errorExtendedPrintf("%s Record initialization failed: Unknown error.",
        record->name);
    return -1;
  }
}

template<typename RecordDeviceSupportType>
long getInterruptInfo(int command, dbCommon *record, ::IOSCANPVT *iopvt) {
  if (!record) {
    errorExtendedPrintf(
        "Configuring I/O Intr support failed: Pointer to record structure is null.");
    *iopvt = nullptr;
    return -1;
  }
  try {
    RecordDeviceSupportType *deviceSupport =
        static_cast<RecordDeviceSupportType *>(record->dpvt);
    if (!deviceSupport) {
      throw std::runtime_error(
          "Pointer to device support data structure is null.");
    }
    deviceSupport->getInterruptInfo(command, iopvt);
  } catch (std::exception &e) {
    errorExtendedPrintf("%s Configuring I/O Intr support failed: %s",
        record->name, e.what());
    return -1;
  } catch (...) {
    errorExtendedPrintf(
        "%s Configuring I/O Intr support failed failed: Unknown error.",
        record->name);
    return -1;
  }
  return 0;
}

template<typename RecordDeviceSupportType>
long processRecord(void *recordVoid) {
  if (!recordVoid) {
    errorExtendedPrintf(
        "Record processing failed: Pointer to record structure is null.");
    return -1;
  }
  dbCommon *record = static_cast<dbCommon *>(recordVoid);
  try {
    RecordDeviceSupportType *deviceSupport =
        static_cast<RecordDeviceSupportType *>(record->dpvt);
    if (!deviceSupport) {
      throw std::runtime_error(
          "Pointer to device support data structure is null.");
    }
    deviceSupport->processRecord();
  } catch (std::exception &e) {
    errorExtendedPrintf("%s Record processing failed: %s", record->name,
        e.what());
    recGblSetSevr(record, SOFT_ALARM, INVALID_ALARM);
    return -1;
  } catch (...) {
    errorExtendedPrintf("%s Record processing failed: Unknown error.",
        record->name);
    recGblSetSevr(record, SOFT_ALARM, INVALID_ALARM);
    return -1;
  }
  return 0;
}

}

extern "C" {

/**
 * Type alias for the get_ioint_info functions. These functions have a slightly
 * different signature than the other functions, even though the definition in
 * the structures in the record header files might indicate something else.
 */
typedef long (*DEVSUPFUN_GET_IOINT_INFO)(int, ::dbCommon *, ::IOSCANPVT *);

/**
 * ai record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
  DEVSUPFUN special_linconv;
} devAiMrf = { 6, nullptr, nullptr, initRecord<MrfAiRecord, ::aiRecord>,
    nullptr, processRecord<MrfAiRecord>, nullptr };
epicsExportAddress(dset, devAiMrf)
;

/**
 * ao record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN write;
  DEVSUPFUN special_linconv;
} devAoMrf = { 6, nullptr, nullptr, initRecord<MrfAoRecord, ::aoRecord>,
    nullptr, processRecord<MrfAoRecord>, nullptr };
epicsExportAddress(dset, devAoMrf)
;

/**
 * bi record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devBiMrf = { 5, nullptr, nullptr, initRecord<MrfBiRecord, ::biRecord>,
    nullptr, processRecord<MrfBiRecord> };
epicsExportAddress(dset, devBiMrf)
;

/**
 * bi record type. Special version for handling interrupts.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devBiInterruptMrf = { 5, nullptr, nullptr, initRecord<MrfBiInterruptRecord,
    ::biRecord>, getInterruptInfo<MrfBiInterruptRecord>, processRecord<
    MrfBiInterruptRecord> };
epicsExportAddress(dset, devBiInterruptMrf)
;

/**
 * bo record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN write;
} devBoMrf = { 5, nullptr, nullptr, initRecord<MrfBoRecord, ::boRecord>,
    nullptr, processRecord<MrfBoRecord> };
epicsExportAddress(dset, devBoMrf)
;

/**
 * longin record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devLonginMrf = { 5, nullptr, nullptr, initRecord<MrfLonginRecord,
    ::longinRecord>, nullptr, processRecord<MrfLonginRecord> };
epicsExportAddress(dset, devLonginMrf)
;

/**
 * longin record type. Special version for handling interrupts.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devLonginInterruptMrf = { 5, nullptr, nullptr, initRecord<
    MrfLonginInterruptRecord, ::longinRecord>, getInterruptInfo<
    MrfLonginInterruptRecord>, processRecord<MrfLonginInterruptRecord> };
epicsExportAddress(dset, devLonginInterruptMrf)
;

/**
 * longout record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN write;
} devLongoutMrf = { 5, nullptr, nullptr, initRecord<MrfLongoutRecord,
    ::longoutRecord>, nullptr, processRecord<MrfLongoutRecord> };
epicsExportAddress(dset, devLongoutMrf)
;

/**
 * longout record type. Special version used for tuning output fine delays.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN write;
} devLongoutFineDelayShiftRegisterMrf = { 5, nullptr, nullptr, initRecord<
    MrfLongoutFineDelayShiftRegisterRecord, ::longoutRecord>, nullptr,
    processRecord<MrfLongoutFineDelayShiftRegisterRecord> };
epicsExportAddress(dset, devLongoutFineDelayShiftRegisterMrf)
;

/**
 * mbbiDirect record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devMbbiDirectMrf = { 5, nullptr, nullptr, initRecord<MrfMbbiDirectRecord,
    ::mbbiDirectRecord>, nullptr, processRecord<MrfMbbiDirectRecord> };
epicsExportAddress(dset, devMbbiDirectMrf)
;

/**
 * mbbiDirect record type. Special version for handling interrupts.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devMbbiDirectInterruptMrf = { 5, nullptr, nullptr, initRecord<
    MrfMbbiDirectInterruptRecord, ::mbbiDirectRecord>, getInterruptInfo<
    MrfMbbiDirectInterruptRecord>, processRecord<MrfMbbiDirectInterruptRecord> };
epicsExportAddress(dset, devMbbiDirectInterruptMrf)
;

/**
 * mbboDirect record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN write;
} devMbboDirectMrf = { 5, nullptr, nullptr, initRecord<MrfMbboDirectRecord,
    ::mbboDirectRecord>, nullptr, processRecord<MrfMbboDirectRecord> };
epicsExportAddress(dset, devMbboDirectMrf)
;

/**
 * mbbi record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devMbbiMrf = { 5, nullptr, nullptr, initRecord<MrfMbbiRecord, ::mbbiRecord>,
    nullptr, processRecord<MrfMbbiRecord> };
epicsExportAddress(dset, devMbbiMrf)
;

/**
 * mbbo record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN write;
} devMbboMrf = { 5, nullptr, nullptr, initRecord<MrfMbboRecord, ::mbboRecord>,
    nullptr, processRecord<MrfMbboRecord> };
epicsExportAddress(dset, devMbboMrf)
;

/**
 * waveform (input) record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN read;
} devWaveformInMrf = { 5, nullptr, nullptr, initRecord<MrfWaveformInRecord,
    ::waveformRecord>, nullptr, processRecord<MrfWaveformInRecord> };
epicsExportAddress(dset, devWaveformInMrf)
;

/**
 * waveform (output) record type.
 */
struct {
  long numberOfFunctionPointers;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN_GET_IOINT_INFO get_ioint_info;
  DEVSUPFUN write;
} devWaveformOutMrf = { 5, nullptr, nullptr, initRecord<MrfWaveformOutRecord,
    ::waveformRecord>, nullptr, processRecord<MrfWaveformOutRecord> };
epicsExportAddress(dset, devWaveformOutMrf)
;

}
