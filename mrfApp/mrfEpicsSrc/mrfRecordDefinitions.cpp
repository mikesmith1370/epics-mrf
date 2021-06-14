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

template<typename RecordDeviceSupportType>
long initRecord(void *recordVoid) {
  if (!recordVoid) {
    errorExtendedPrintf(
        "Record initialization failed: Pointer to record structure is null.");
    return -1;
  }
  dbCommon *record = static_cast<dbCommon *>(recordVoid);
  try {
    RecordDeviceSupportType *deviceSupport = new RecordDeviceSupportType(
      static_cast<typename RecordDeviceSupportType::RecordType *>(
        recordVoid));
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
long processRecord(typename RecordDeviceSupportType::RecordType *record) {
  if (!record) {
    errorExtendedPrintf(
        "Record processing failed: Pointer to record structure is null.");
    return -1;
  }
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
#ifndef HAS_aidset
typedef struct aidset {
  dset common;
  long (*read_ai)(aiRecord *prec);
  long (*special_linconv)(aiRecord *prec, int after);
} aidset;
#endif // HAS_aidset
aidset devAiMrf = {
  {
    6,
    nullptr,
    nullptr,
    initRecord<MrfAiRecord>,
    nullptr,
  },
  processRecord<MrfAiRecord>,
  nullptr,
};
epicsExportAddress(dset, devAiMrf);

/**
 * ao record type.
 */
#ifndef HAS_aodset
typedef struct aodset {
  dset common;
  long (*write_ao)(aiRecord *prec);
  long (*special_linconv)(aiRecord *prec, int after);
} aodset;
#endif // HAS_aodset
aodset devAoMrf = {
  {
    6,
    nullptr,
    nullptr,
    initRecord<MrfAoRecord>,
    nullptr,
  },
  processRecord<MrfAoRecord>,
  nullptr,
};
epicsExportAddress(dset, devAoMrf);

/**
 * bi record type.
 */
#ifndef HAS_bidset
typedef struct bidset {
  dset common;
  long (*read_bi)(biRecord *prec);
} bidset;
#endif // HAS_bidset
bidset devBiMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfBiRecord>,
    nullptr,
  },
  processRecord<MrfBiRecord>,
};
epicsExportAddress(dset, devBiMrf);

/**
 * bi record type. Special version for handling interrupts.
 */
bidset devBiInterruptMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfBiInterruptRecord>,
    reinterpret_cast<DEVSUPFUN>(getInterruptInfo<MrfBiInterruptRecord>),
  },
  processRecord<MrfBiInterruptRecord>,
};
epicsExportAddress(dset, devBiInterruptMrf);

/**
 * bo record type.
 */
#ifndef HAS_bodset
typedef struct bodset {
  dset common;
  long (*write_bo)(boRecord *prec);
} bodset;
#endif // HAS_bodset
bodset devBoMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfBoRecord>,
    nullptr,
  },
  processRecord<MrfBoRecord>,
};
epicsExportAddress(dset, devBoMrf);

/**
 * longin record type.
 */
#ifndef HAS_longindset
typedef struct longindset {
  dset common;
  long (*read_longin)(longinRecord *prec);
} longindset;
#endif // HAS_longindset
longindset devLonginMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfLonginRecord>,
    nullptr,
  },
  processRecord<MrfLonginRecord>,
};
epicsExportAddress(dset, devLonginMrf);

/**
 * longin record type. Special version for handling interrupts.
 */
longindset devLonginInterruptMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfLonginInterruptRecord>,
    reinterpret_cast<DEVSUPFUN>(getInterruptInfo<MrfLonginInterruptRecord>),
  },
  processRecord<MrfLonginInterruptRecord>,
};
epicsExportAddress(dset, devLonginInterruptMrf);

/**
 * longout record type.
 */
#ifndef HAS_longoutdset
typedef struct longoutdset {
  dset common;
  long (*write_longout)(longoutRecord *prec);
} longoutdset;
#endif // HAS_longoutdset
longoutdset devLongoutMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfLongoutRecord>,
    nullptr,
  },
  processRecord<MrfLongoutRecord>,
};
epicsExportAddress(dset, devLongoutMrf);

/**
 * longout record type. Special version used for tuning output fine delays.
 */
longoutdset devLongoutFineDelayShiftRegisterMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfLongoutFineDelayShiftRegisterRecord>,
    nullptr,
  },
  processRecord<MrfLongoutFineDelayShiftRegisterRecord>,
};
epicsExportAddress(dset, devLongoutFineDelayShiftRegisterMrf);

/**
 * mbbiDirect record type.
 */
#ifndef HAS_mbbidirectdset
typedef struct mbbidirectdset {
  dset common;
  long (*read_mbbi)(mbbiDirectRecord *prec);
} mbbidirectdset;
#endif // HAS_mbbidirectdset
mbbidirectdset devMbbiDirectMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfMbbiDirectRecord>,
    nullptr,
  },
  processRecord<MrfMbbiDirectRecord>,
};
epicsExportAddress(dset, devMbbiDirectMrf);

/**
 * mbbiDirect record type. Special version for handling interrupts.
 */
mbbidirectdset devMbbiDirectInterruptMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfMbbiDirectInterruptRecord>,
    reinterpret_cast<DEVSUPFUN>(getInterruptInfo<MrfMbbiDirectInterruptRecord>),
  },
  processRecord<MrfMbbiDirectInterruptRecord>,
};
epicsExportAddress(dset, devMbbiDirectInterruptMrf);

/**
 * mbboDirect record type.
 */
#ifndef HAS_mbbodirectdset
typedef struct mbbodirectdset {
  dset common;
  long (*write_mbbo)(mbboDirectRecord *prec);
} mbbodirectdset;
#endif // HAS_mbbodirectdset
mbbodirectdset devMbboDirectMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfMbboDirectRecord>,
    nullptr,
  },
  processRecord<MrfMbboDirectRecord>,
};
epicsExportAddress(dset, devMbboDirectMrf);

/**
 * mbbi record type.
 */
#ifndef HAS_mbbidset
typedef struct mbbidset {
  dset common;
  long (*read_mbbi)(mbbiRecord *prec);
} mbbidset;
#endif // HAS_mbbidset
mbbidset devMbbiMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfMbbiRecord>,
    nullptr,
  },
  processRecord<MrfMbbiRecord>,
};
epicsExportAddress(dset, devMbbiMrf);

/**
 * mbbo record type.
 */
#ifndef HAS_mbbodset
typedef struct mbbodset {
  dset common;
  long (*write_mbbo)(mbboRecord *prec);
} mbbodset;
#endif // HAS_mbbodset
mbbodset devMbboMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfMbboRecord>,
    nullptr,
  },
  processRecord<MrfMbboRecord>,
};
epicsExportAddress(dset, devMbboMrf);

/**
 * waveform (input) record type.
 */
#ifndef HAS_wfdset
typedef struct wfdset {
  dset common;
  long (*read_wf)(waveformRecord *prec);
} wfdset;
#endif // HAS_wfdset
wfdset devWaveformInMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfWaveformInRecord>,
    nullptr,
  },
  processRecord<MrfWaveformInRecord>,
};
epicsExportAddress(dset, devWaveformInMrf);

/**
 * waveform (output) record type.
 */
wfdset devWaveformOutMrf = {
  {
    5,
    nullptr,
    nullptr,
    initRecord<MrfWaveformOutRecord>,
    nullptr,
  },
  processRecord<MrfWaveformOutRecord>,
};
epicsExportAddress(dset, devWaveformOutMrf);

} // extern "C"
