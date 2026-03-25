#!/usr/bin/env python3

import sys
import itertools

ecs = [
    ("ENABLE_EC_SYS", "ec_sys"),
    ("ENABLE_EC_ACPI", "acpi_ec"),
    ("ENABLE_EC_DEV_PORT", "dev_port"),
    ("ENABLE_EC_DUMMY", "dummy"),
]

print("/* Auto-generated code: %r */" % sys.argv)
print()
print('#ifndef NBFC_EC_AVAILABLE_H_')
print('#define NBFC_EC_AVAILABLE_H_')
print()
print('#include "config.h"')
print()

first = True

# generate all non-empty combinations
for r in range(len(ecs), 0, -1):
    for combo in itertools.combinations(ecs, r):
        cond = " && ".join(m for m, _ in combo)
        names = ", ".join(n for _, n in combo)

        if first:
            print(f"#if {cond}")
            first = False
        else:
            print(f"#elif {cond}")

        print(f'#define EC_AVAILABLE_STR "{names}"\n')

print("#else")
print('#define EC_AVAILABLE_STR ""')
print("#endif")
print()
print('#endif')
