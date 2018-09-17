Installing the device support
=============================

Installing the MRF device support is pretty simple as it has a very small set of
dependencies.


Requirements
------------

- EPICS Base (3.14 or newer)
- C++ 11 compliant compiler and standard library


Compiling
---------

Create the file `configure/RELEASE.local` and add the path to your EPICS Base
installation. Example:

```
EPICS_BASE = /path/to/my/epics/base-3.15.5
```

Depending on your default compiler options, you might also have to create the
file `configure/CONFIG_SITE.local` and add some compiler flags (to enable C++ 11
support or other features). Example:

```
USR_CXXFLAGS += -std=c++11
USR_CFLAGS += -std=c99
```

After this, running `make` should be sufficient for building the device support.
Please note that the support for controlling EVG / EVR devices that are directly
installed in the computer running the IOC (e.g. devices for Compact PCI or
MicroTCA) is only compiled on Linux as other operating systems are currently not
supported for these kind of devices.

When you want to use these devices, you also have to install the Linux kernel
module, which can be found in the `support` directory of the device support.
