#!/usr/bin/env python3

# Copyright 2021 aquenos GmbH.
# Copyright 2021 Karlsruhe Institute of Technology.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#
# This software has been developed by aquenos GmbH on behalf of the
# Karlsruhe Institute of Technology's Institute for Beam Physics and
# Technology.

"""
Generate code for preheating the memory cache for a device.

This script is intended for use by the device support maintainers, so that it
is easier to generate the code that preheats the cache for a specific device
type, thus reducing the startup times of IOCs that control more than one
device.

This script expects the output of the mrfDumpCache command on standard input
and prints the appropriate generated code on standard output.
"""

import enum
import re
import sys


class _Section(enum.Enum):
    UINT16 = 1
    UINT32 = 2


_regexp_for_section = {
    _Section.UINT16: re.compile("(0x[0-9a-f]{8}): 0x[0-9a-f]{4}"),
    _Section.UINT32: re.compile("(0x[0-9a-f]{8}): 0x[0-9a-f]{8}"),
}


def _generate_code(start_address, block_length, section_type):
    # If there are more than two consecutive registers, we generate a for loop
    # because this is the more compact representation.
    if section_type == _Section.UINT16:
        if block_length > 4:
            print(
                "  for (std::uint32_t address = 0x{:08x}; "
                "address < 0x{:08x}; address += 2) {{".format(
                    start_address, start_address + block_length
                )
            )
            print("      cache->tryCacheUInt16(address);")
            print("  }")
        else:
            for address in range(
                start_address, start_address + block_length, 2
            ):
                print("  cache->tryCacheUInt16(0x{:08x});".format(address))
    elif section_type == _Section.UINT32:
        if block_length > 8:
            print(
                "  for (std::uint32_t address = 0x{:08x}; "
                "address < 0x{:08x}; address += 4) {{".format(
                    start_address, start_address + block_length
                )
            )
            print("      cache->tryCacheUInt32(address);")
            print("  }")
        else:
            for address in range(
                start_address, start_address + block_length, 4
            ):
                print("  cache->tryCacheUInt32(0x{:08x});".format(address))


def main():
    block_length = 0
    current_section = None
    line_count = 1
    register_length = 0
    start_address = None
    for line in sys.stdin:
        line = line.strip()
        if not line:
            pass
        elif line == "uint16 registers:":
            if start_address is not None:
                _generate_code(start_address, block_length, current_section)
                start_address = None
            current_section = _Section.UINT16
            register_length = 2
        elif line == "uint32 registers:":
            if start_address is not None:
                _generate_code(start_address, block_length, current_section)
                start_address = None
            current_section = _Section.UINT32
            register_length = 4
        else:
            if current_section:
                match = _regexp_for_section[current_section].fullmatch(line)
            else:
                match = None
            if match:
                address = int(match.group(1), 16)
                if start_address is None:
                    start_address = address
                    block_length = register_length
                elif address == (start_address + block_length):
                    block_length += register_length
                else:
                    _generate_code(
                        start_address, block_length, current_section
                    )
                    start_address = address
                    block_length = register_length
            else:
                raise Exception(
                    "Invalid input in line {}: {}".format(line_count, line))
        line_count += 1
    if start_address is not None:
        _generate_code(start_address, block_length, current_section)


if __name__ == "__main__":
    main()
