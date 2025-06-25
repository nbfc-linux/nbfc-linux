#!/usr/bin/env python3

'''Extract named register fields from a DSDT DSL file'''

import re
import argparse

class NamedField:
    def __init__(self, name, bit_offset, bit_length):
        self.name        = name
        self.bit_length  = bit_length
        self.byte_offset = bit_offset // 8
        self.bit_offset  = bit_offset %  8
        self.mask        = ((1 << self.bit_length) - 1) << self.bit_offset

    def __repr__(self):
        return '%-4s  byte_offset=%-3d  bit_offset=%-2d  bit_length=%-2d  mask=0x%02X' % (
            self.name, self.byte_offset, self.bit_offset, self.bit_length, self.mask)

def parse_dsl_embeddedcontrol(dsl_code):
    results = []
    lines = dsl_code.splitlines()
    inside_ec_region = False
    bit_offset = 0

    for line in lines:
        line = line.strip()

        # Start EmbeddedControl region
        if re.match(r'^OperationRegion\s*\(\s*\w+,\s*EmbeddedControl\s*,', line):
            inside_ec_region = True
            continue

        # End of Field definition
        if inside_ec_region and line.startswith('}'):
            inside_ec_region = False
            byte_offset = 0
            bit_offset = 0
            continue

        if inside_ec_region:
            # Match Offset directive
            offset_match = re.match(r'^Offset\s*\(\s*(0x[0-9A-Fa-f]+|\d+)\s*\)', line)
            if offset_match:
                bit_offset = int(offset_match.group(1), 0) * 8
                continue

            # Match anonymous field
            anon_match = re.match(r'^,\s*(\d+)', line)
            if anon_match:
                bits = int(anon_match.group(1))
                bit_offset += bits
                continue

            # Match named field
            named_match = re.match(r'^(\w+)\s*,\s*(\d+)', line)
            if named_match:
                name = named_match.group(1)
                bits = int(named_match.group(2))
                results.append(NamedField(name, bit_offset, bits))
                bit_offset += bits
                continue

    return results

def analyze(opts):
    with open(opts.DSL_FILE,'r', encoding='UTF-8') as fh:
        content = fh.read()

    fields = parse_dsl_embeddedcontrol(content)

    for field in fields:
        print(field)

epilog = '''\
To get the DSDT DSL File follow these steps:

  - Install the `iasl` program:
    - Debian: sudo apt install acpica-tools
    - Fedora: sudo dnf install acpica-tools
    - Arch Linux: sudo pacman -S acpica
  
  - Extract the binary ACPI tables:
    sudo cat /sys/firmware/acpi/tables/DSDT > dsdt.dat
  
  - Generate the DSL file:
    iasl -d dsdt.dat
  
  - The dsl file is now available as `dsdt.dsl`\
'''

argp = argparse.ArgumentParser(
    description=__doc__,
    epilog=epilog,
    formatter_class=argparse.RawDescriptionHelpFormatter)
argp.add_argument('DSL_FILE')
opts = argp.parse_args()
analyze(opts)
