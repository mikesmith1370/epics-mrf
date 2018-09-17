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

#ifndef ANKA_MRF_FD_SELECTOR_H
#define ANKA_MRF_FD_SELECTOR_H

extern "C" {
#include <sys/select.h>
}

namespace anka {
namespace mrf {

/**
 * Helper class for having a select operation that can be interrupted by another
 * thread. This is implemented through a pipe that the select operation waits
 * on.
 */
class MrfFdSelector {

public:

  /**
   * Default constructor. Creates the pipe that is internally used for waking up
   * from the select operation. Throws an exception if the pipe cannot be
   * created.
   */
  MrfFdSelector();

  /**
   * Destructor. Closes the pipe that has been created for waking up from the
   * select operation. Destroying the selector while a select operation is in
   * progress results in undefined behavior.
   */
  ~MrfFdSelector();

  /**
   * Waits for a file-descriptor related event to happen. This method delegates
   * to the {@code select} function defined by the POSIX API, so its arguments
   * have exactly the same meaning as outlined in the documentation of the
   * POSIX API. Unlike the {@code select} function from the POSIX API, this
   * method takes the greatest file-descriptor  that is part of any of the sets
   * as a parameter and not the number of file descriptors (which would be
   * {@code maxFd + 1}). If the {@code select} call fails, this method throws an
   * exception.
   *
   * The main difference to calling {@code select} from the POSIX API directly
   * is that this method adds the file descriptor for the read-side of the
   * internal pipe to the set of {@code readFds} and increases {@code maxFd} if
   * it is less than this file descriptor plus one. This internal file
   * descriptor is removed from the set before returning, so the calling code
   * will never see its bit set in {@code readFds}.
   *
   * Adding this file descriptor to the set of read file-descriptors has the
   * effect that a call to {@link wakeUp()} will cause the {@code select}
   * operation to return immediately. This is internally archived by writing
   * to the pipe so that {@code select} detects that the read file-descriptor
   * of the pipe is ready for reading data.
   */
  void select(::fd_set *readFds, ::fd_set *writeFds, ::fd_set *errorFds,
      int maxFd, ::timeval *timeout);

  /**
   * Wakes a thread that is currently waiting on a {@code select} operation up.
   * This means that the {@code select} operation returns immediately without
   * waiting any longer. If no thread is currently waiting on a {@code select}
   * operation, the next time {@code select} is called it will return
   * immediately instead of waiting for an external event. If the write
   * operation that is needed to wake up from the {@code select} call fails,
   * this method throws an exception.
   */
  void wakeUp();

private:

  int readFd = -1;
  int writeFd = -1;

};

} //namespace mrf
} //namespace anka

#endif // ANKA_MRF_FD_SELECTOR_H
