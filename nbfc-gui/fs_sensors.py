#!/usr/bin/env python3

import os
from collections import namedtuple

LinuxHwmonDirs = [
  "/sys/class/hwmon/hwmon%d",
  "/sys/class/hwmon/hwmon%d/device",
]

LinuxTempSensorFile = "temp%d_input"

SensorFile = namedtuple('SensorFile', ['name', 'file'])

def get_sensors():
    result = []

    for hwmonDir in LinuxHwmonDirs:
        for i in range(10):
            dir = hwmonDir % i
            file = "%s/name" % dir

            try:
                with open(file, 'r') as fh:
                    source_name = fh.read().strip()
            except:
                continue

            for j in range(10):
                filename = LinuxTempSensorFile % j
                filename = "%s/%s" % (dir, filename)

                if os.path.exists(filename):
                    result.append(SensorFile(source_name, filename))

    return result
