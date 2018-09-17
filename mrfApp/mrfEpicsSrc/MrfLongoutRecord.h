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

#ifndef ANKA_MRF_EPICS_LONGOUT_RECORD_H
#define ANKA_MRF_EPICS_LONGOUT_RECORD_H

#include <longoutRecord.h>

#include "MrfOutputRecord.h"

namespace anka {
namespace mrf {
namespace epics {

/**
 * Device support class for the ao record.
 */
class MrfLongoutRecord: public MrfOutputRecord<::longoutRecord> {

public:

  /**
   * Creates an instance of the device support for the specified record.
   */
  MrfLongoutRecord(::longoutRecord *record);

protected:

  std::uint32_t readRecordValue();

  void writeRecordValue(std::uint32_t value);

private:

  // We do not want to allow copy or move construction or assignment.
  MrfLongoutRecord(const MrfLongoutRecord &) = delete;
  MrfLongoutRecord(MrfLongoutRecord &&) = delete;
  MrfLongoutRecord &operator=(const MrfLongoutRecord &) = delete;
  MrfLongoutRecord &operator=(MrfLongoutRecord &&) = delete;

};

}
}
}

#endif // ANKA_MRF_EPICS_LONGOUT_RECORD_H
