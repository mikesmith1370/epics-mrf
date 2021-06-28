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


Choosing the record type for descriptions
-----------------------------------------

There are records that can store a user-defined description for many settings
(e.g. for each input, output, pulse generator, prescaler, etc.). At compile
time, you can choose the record type used for these records. You have a choice
between three types, that can be selected by setting the
`MRF_DESCRIPTION_RECORD_TYPE` variable in `configure/CONFIG_SITE.local` to the
name of the respective record type.

### stringout

This is the default option and works with all supported releases of EPICS Base.
The only downside of this option is that description strings are limited to 39
characters.

### lso

When choosing the `lso` record, descriptions can have up to 255 characters. The
downside is that the `lso` record is only supported by recent releases of EPICS
Base and that there is a
[bug in Autosave](https://github.com/epics-modules/autosave/pull/27) that
prevents values from being restored correctly unless you apply a patch to
Autosave.

### waveform

The `waveform` record is a compromise. When using it, you can use up to 255
characters for descriptions, like when using the `lso` record, but the
`waveform` record should work with all supported releases of EPICS Base.
The downside of using the `waveform` record is that EPICS client tools will not
natively recognize that the PV represents a string and that the format in which
Autosave stores the data differs from the one used for the `stringout` and `lso`
records. This means that you will have to manually edit Autosave save files when
switching between `waveform` and one of the other record types. Another downside
is that the CSS / BOY panels, that ship with this device support, will not work
when using the `waveform` record type. The panels have to be adjusted first
(so that they reference the record’s `VAL` field instead of `VAL$`).
