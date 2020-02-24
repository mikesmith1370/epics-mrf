Configuring the device support
==============================

In order to use the device support in an EPICS IOC, the IOC has to include one
of the following DBD files:

- `mrfMmap.dbd` (only when controlling a device over PCI(e))
- `mrfUdpIp.dbd` (only when controlling a device over Ethernet)

The IOC has to be linked against the following libraries:

- `mrfCommon`
- `mrfEpics`
- `mrfEpicsMmap` and `mrfMmap` (only when controlling a device over PCI(e))
- `mrfEpicsUdpIp` and `mrfUdpIp` (only when controlling a device over Ethernet)


### Table of contents
- [IOC startup configuration](#ioc-startup-configuration)
- [Autosave support](#autosave-support)
- [Interrupt handling](#interrupt-handling)
- [GUI / OPI panels](#gui--opi-panels)


IOC startup configuration
-------------------------

The IOC startup script has to connect to the device(s) and load the
corresponding database files. Both parts depend on the type of the device.


### VME-EVG-230

```
mrfUdpIpEvgDevice("EVG01", "evg.example.com")

dbLoadRecords("${MRF}/db/vme-evg-230.db", "P=TEST:EVG:,R=01:,DEVICE=EVG01,UNIV_OUT_0_1_INSTALLED=0,UNIV_OUT_2_3_INSTALLED=0,UNIV_IN_0_1_INSTALLED=0,UNIV_IN_2_3_INSTALLED=0,TB_UNIV_IN_0_1_INSTALLED=0,TB_UNIV_IN_2_3_INSTALLED=0,TB_UNIV_IN_4_5_INSTALLED=0,TB_UNIV_IN_6_7_INSTALLED=0,TB_UNIV_IN_8_9_INSTALLED=0,TB_UNIV_IN_10_11_INSTALLED=0,TB_UNIV_IN_12_13_INSTALLED=0,TB_UNIV_IN_14_15_INSTALLED=0")
```

The following parameters have to be passed to `dbLoadRecords`:

<table>
<tr>
<th>Parameter Name</th>
<th>Description</th>
</tr>
<tr>
<td><code>P</code></td>
<td>First part of the prefix for all EPICS PVs.</td>
</tr>
<tr>
<td><code>R</code></td>
<td>Second part of the prefix for all EPICS PVs (e.g. device number).</td>
</tr>
<tr>
<td><code>DEVICE</code></td>
<td>Name of the device (must be the name that is specified when calling <code>mrfUdpIpEvgDevice</code>).</td>
</tr>
</table>

The following parameters are optional. If they are not specified, the respective
default values are used.

<table>
<tr>
<th>Parameter Name</th>
<th>Description</th>
<th>Default Value</th>
</tr>
<tr>
<td><code>UNIV_OUT_x_y_INSTALLED</code></td>
<td><code>0</code> if the corresponding universal output module is not installed, <code>1</code> if it is installed.</td>
<td><code>0</code></td>
</tr>
<tr>
<td><code>UNIV_IN_x_y_INSTALLED</code></td>
<td><code>0</code> if the corresponding universal input module (front panel) is not installed, <code>1</code> if it is installed.</td>
<td><code>0</code></td>
</tr>
<tr>
<td><code>TB_UNIV_IN_x_y_INSTALLED</code></td>
<td><code>0</code> if the corresponding universal input module (transition board) is not installed, <code>1</code> if it is installed.</td>
<td><code>0</code></td>
</tr>
</table>


### VME-EVR-230RF

```
mrfUdpIpEvrDevice("EVR01", "evr.example.com")

dbLoadRecords("${MRF}/db/vme-evr-230rf.db","P=TEST:EVR:,R=01:,DEVICE=EVR01,UNIV_OUT_0_1_INSTALLED=0,UNIV_OUT_2_3_INSTALLED=0,UNIV_OUT_0_1_FD_AVAILABLE=0,UNIV_OUT_2_3_FD_AVAILABLE=0,TB_UNIV_OUT_0_1_INSTALLED=0,TB_UNIV_OUT_2_3_INSTALLED=0,TB_UNIV_OUT_4_5_INSTALLED=0,TB_UNIV_OUT_6_7_INSTALLED=0,TB_UNIV_OUT_8_9_INSTALLED=0,TB_UNIV_OUT_10_11_INSTALLED=0,TB_UNIV_OUT_12_13_INSTALLED=0,TB_UNIV_OUT_14_15_INSTALLED=0")
```

The following parameters have to be passed to `dbLoadRecords`:

<table>
<tr>
<th>Parameter Name</th>
<th>Description</th>
</tr>
<tr>
<td><code>P</code></td>
<td>First part of the prefix for all EPICS PVs.</td>
</tr>
<tr>
<td><code>R</code></td>
<td>Second part of the prefix for all EPICS PVs (e.g. device number).</td>
</tr>
<tr>
<td><code>DEVICE</code></td>
<td>Name of the device (must be the name that is specified when calling <code>mrfUdpIpEvrDevice</code>).</td>
</tr>
</table>

The following parameters are optional. If they are not specified, the respective
default values are used.

<table>
<tr>
<th>Parameter Name</th>
<th>Description</th>
<th>Default Value</th>
</tr>
<tr>
<td><code>UNIV_OUT_x_y_INSTALLED</code></td>
<td><code>0</code> if the corresponding universal output module (front panel) is not installed, <code>1</code> if it is installed.</td>
<td><code>0</code></td>
</tr>
<tr>
<td><code>UNIV_OUT_x_y_FD_AVAILABLE</code></td>
<td><code>0</code> if the corresponding universal output module (front panel) is not installed or does not allow specifying a fine delay, <code>1</code> if it is installed and allows specifying a fine delay.</td>
<td><code>0</code></td>
</tr>
<tr>
<td><code>TB_UNIV_OUT_x_y_INSTALLED</code></td>
<td><code>0</code> if the corresponding universal output module (transition board) is not installed, <code>1</code> if it is installed.</td>
<td><code>0</code></td>
</tr>
</table>


### MTCA-EVR-300

```
mrfMmapMtcaEvr300Device("EVR01", "/dev/era3")

dbLoadRecords("${MRF}/db/mtca-evr-300.db","P=TEST:EVR:,R=01:,DEVICE=EVR01")
```

If there is more than one EVR module in the system, the path to the device node
might have to be changed (e.g. `/dev/erb3`, `/dev/erc3`, etc.).

The following parameters have to be passed to `dbLoadRecords`:

<table>
<tr>
<th>Parameter Name</th>
<th>Description</th>
</tr>
<tr>
<td><code>P</code></td>
<td>First part of the prefix for all EPICS PVs.</td>
</tr>
<tr>
<td><code>R</code></td>
<td>Second part of the prefix for all EPICS PVs (e.g. device number).</td>
</tr>
<tr>
<td><code>DEVICE</code></td>
<td>Name of the device (must be the name that is specified when calling <code>mrfMmapMtcaEvr300Device</code>).</td>
</tr>
</table>


### Supplemental database files

There are two additional database files that can be used for each device,
regardless of the device type:

 - `mrf-autosave-menu.db`
 - `mrf-write-all-settings-with-status.db`

The `mrf-autosave-menu.db` provides integration with
[Autosave](https://github.com/epics-modules/autosave) for managing the device
configuration (see below).

The `mrf-write-all-settings-with-status.db` provides two additional records that
can be used to write the current value of all settings to the hardware. This is
particularly useful with the VME-based hardware that might be restarted without
restarting the EPICS IOC. The basic support for writing all settings is already
provided by the standard database files (it can be triggered by writing to
`$(P)$(R)WriteAll.PROC`). The additional database file adds two more records
that provide make it possible to see whether the operation has finished. When
using this database file, one writes to `$(P)$(R)WriteAll:DoWithStatus.PROC` and
monitors `$(P)$(R)WriteAll:Status`, which is `1` while the operation is in
progress and `0` when it has finished.

These records are not included in the standard database file because they
require the `sseq` record which is not part of EPICS Base, but is provided by
the [calc module](https://github.com/epics-modules/calc).

When loading the database file, the `P` and `R` parameters have to be set to the
same values that are specified for the primary database file of the respective
device.


Autosave support
----------------

The [Autosave](https://github.com/epics-modules/autosave) support is entirely
optional and does not have to be used. For each device type, there is an
Autosave request file that lists all records storing settings for the respective
device type. As Autosave uses the name of the request file for constructing the
names of the save files, it is strongly suggested to create a custom request
file that includes the generic request file.

The `mrf-autosave-menu.db` database file can be used for loading and saving
configuration sets at runtime (see "Supplemental database files" above).


### Example

For saving a VME-EVR-230RF device, one could use the following request file (in
this example, we call it `evr01_preset.req`):

```
file vme-evr-230rf.req P=$(P),R=$(R)
$(P)$(R)Presets:Intrnl:Menu:currDesc
```

The second line is only needed if the support for loading and saving configuration sets (`mrf-autosave-menu.db`) is supposed to be used.

The corresponding IOC startup file would contain the following lines:

```
# Before iocInit
dbLoadRecords("${MRF}/db/mrf-autosave-menu.db", "P=TEST:EVR:,R=01:,CONFIG_PV_PREFIX=Presets:,CONFIG_NAME=evr01_preset")
dbLoadRecords("${MRF}/db/mrf-write-all-settings-with-status.db", "P=TEST:EVR:,R=01:")
set_requestfile_path("${TOP}/db", "")
set_requestfile_path("${MRF}/db", "")
set_savefile_path("${TOP}/iocBoot/${IOC}/autosave")
save_restoreSet_DatedBackupFiles(0)
save_restoreSet_NumSeqFiles(3)
save_restoreSet_SeqPeriodInSeconds(600)
save_restoreSet_CAReconnect(1)
save_restoreSet_IncompleteSetsOk(1)

# After iocInit
create_manual_set("evr02_preset.req", "P=A:TI:EVR:,R=02:,CONFIGMENU=1")
```

If the settings should also be saved automatically, one would use
`create_monitor_set` instead of `create_manual_set` and not specify
`CONFIGMENU=1`. If the settings should only be saved automatically and no manual
configuration management was desired, one would also omit the `dbLoadRecords`
lines.


Interrupt handling
------------------

Interrupts are only supported when using the mmap-based device support (the
UDP/IP-based protocol does not include support for interrupts). In order for an
interrupt to be triggered, it has to be enabled. This can be done through the
following process variables:

 - `$(P)$(R)IRQ:Enabled` (global flag, must be enabled for any of the other
   flags to be effective)
 - `$(P)$(R)IRQ:DataBuffer:Enabled`
 - `$(P)$(R)IRQ:Event:Enabled`
 - `$(P)$(R)IRQ:EventFIFOFull:Enabled`
 - `$(P)$(R)IRQ:Hardware:Enabled`
 - `$(P)$(R)IRQ:Heartbeat:Enabled`
 - `$(P)$(R)IRQ:LinkStateChange:Enabled`
 - `$(P)$(R)IRQ:RXViolation:Enabled`

If a certain interrupt type is not enabled, the corresponding bit in the
interrupt flags will never be set. Even when it is set in the device, it is
filtered by the device support so that only those flags that have been enabled
will actually cause the interrupt handlers to be notified.

Interrupts can be handled in two different ways. They can be mapped to EPICS
events, or processing of a record can be triggered.


### Triggering an EPICS event

A mapping from an interrupt to an EPICS event can be defined with the
`mrfMapInterruptToEvent` IOC shell command:

```
mrfMapInterruptToEvent("EVR01", 5, "0xffffffff")
```

The first parameter is the name of the device, the second the number of the
EPICS event that shall be posted, and the third is the interrupt mask. The
interrupt mask specifies which kind of interrupts trigger the event. Only when
at least one of the bits specified in the mask is also set in the interrupt flag
register, the event is triggered. The meaning of the bits in the interrupt flag
register is documented in the manual. On an EVR, it should typically be as follows:

<table>
<tr>
<th>Bit #</th>
<th>Meaning</th>
</tr>
<tr>
<td>0</td>
<td>Receiver violation flag</td>
</tr>
<tr>
<td>1</td>
<td>Event FIFO full flag</td>
</tr>
<tr>
<td>2</td>
<td>Heartbeat interrupt flag</td>
</tr>
<tr>
<td>3</td>
<td>Event interrupt flag</td>
</tr>
<tr>
<td>4</td>
<td>Hardware interrupt flag (mapped signal)</td>
</tr>
<tr>
<td>5</td>
<td>Data buffer flag</td>
</tr>
<tr>
<td>6</td>	
<td>ink state change interrupt flag</td>
</tr>
<tr>
<td>7 - 31</td>
<td>unused</td>
</tr>
</table>


### Triggering processing of an I/O Intr record

An interrupt can also be used to trigger processing of a record. The interrupt
has to be of type `bi`, `longin`, or `mbbiDirect`. The `DTYP` has to be set to
`MRF Interrupt` and `SCAN` has to be `I/O Intr`. When processed, the record's
`VAL` (`longin`) or `RVAL` (`bi`, `mbbiDirect`) is set to the content of the
interrupt flags register, masked by the specified mask. The mask is specified in
the `INP` field of the record, like in the following example:

```
record(longin, "MyInterrupt") {
  field(DTYP, "MRF Interrupt")
  field(INP,  "@EVR01 interrupt_flags_mask=0x01")
  field(SCAN, "I/O Intr")
}
```

In this example, `EVR01` is the name of the device and the interrupt mask is set
to `0x01`. If no mask is specified, a mask that has all bits set is used.
Processing of the record is only triggered if at least one of the bits specified
in the mask is set in the interrupt flag register.

Such a record can also be used with a different scan mode, but in this case it
is much less useful: When being processed (e.g. periodically or by another
trigger) it will simply get the value that corresponds to the last interrupt.


GUI / OPI panels
----------------

A complete set of panels for controlling the EVG and EVR are distributed with
this device support. This panels are designed to work with
[CSS](http://controlsystemstudio.org/) / BOY and can be found in the `opi/boy`
directory of this device support.

There are four different panels that can be used:


### EVG panel

The main EVG panel is provided by the file `EVG/Main.opi`. When opening this
panel, a couple of macros have to be set:

<table>
<tr>
<th>Macro name</th>
<th>Description</th>
</tr>
<tr>
<td>mrf_evg_name</td>
<td>Human-readable name of the EVG. This is used in the title of the panel.</td>
</tr>
<tr>
<td>mrf_evg_prefix</td>
<td>PV prefix of the EVG. This is the combination of the <code>P</code> and <code>R</code> macros specified in the IOC startup file.</td>
</tr>
<tr>
<td>mrf_evg_type</td>
<td>Type of the EVG. The type is used to determine the features of the EVG and display the respective GUI elements. At the moment, only <code>VME-EVG-230</code> is supported.</td>
</tr>
</table>


### EVR panel

The main EVR panel is provided by the files `EVR/Main_MTCA-EVR-300.opi` and
`EVR/Main_VME-EVR-230RF.opi` (depending on the type of the EVR). When opening
this panel, a couple of macros have to be set:

<table>
<tr>
<th>Macro name</th>
<th>Description</th>
</tr>
<tr>
<td>mrf_evr_name</td>
<td>Human-readable name of the EVR. This is used in the title of the panel.</td>
</tr>
<tr>
<td>mrf_evr_prefix</td>
<td>PV prefix of the EVR. This is the combination of the <code>P</code> and <code>R</code> macros specified in the IOC startup file.</td>
</tr>
</table>


### Autosave configuration management panel

When using [Autosave](#autosave-support), there is a panel for managing
configuration sets saved with Autosave. This panel is provided by the file
`ConfigMenu/MainFull.opi` (or `ConfigMenu/MainLight.opi` for a version that does
not display the description for each saved file).

When opening this panel, a couple of macros have to be set:

<table>
<tr>
<th>Macro name</th>
<th>Description</th>
</tr>
<tr>
<td>mrf_config_menu_device_name</td>
<td>Human-readable name of the EVR. This is used in the title of the panel.</td>
</tr>
<tr>
<td>mrf_config_menu_device_prefix</td>
<td>PV prefix of the EVR. This is the combination of the <code>P</code> and <code>R</code> macros specified in the IOC startup file.</td>
</tr>
<tr>
<td>mrf_config_menu_prefix</td>
<td>PV prefix used for the Autosave PVs. This is the prefix that is specified with the <code>CONFIG_PV_PREFIX</code> macro when loading the <code>mrf-autosave-menu.db</code> file in the IOC startup file.</td>
</tr>
</table>
