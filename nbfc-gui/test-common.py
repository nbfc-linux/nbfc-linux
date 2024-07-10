#!/usr/bin/python

from fs_sensors import *
from common import *

AVAILABLE_SENSORS = [
    SensorFile('coretemp', '/sys/class/hwmon/hwmon1/temp1_input'),
    SensorFile('coretemp', '/sys/class/hwmon/hwmon1/temp2_input'),
]

def test(
    number,
    fan_temperature_sources,
    fan_count,
    available_sensors, 
    expected_errors,
    expected_fixed):

    errors = get_fan_temperature_sources_errors(fan_temperature_sources, fan_count, available_sensors)

    if errors != expected_errors:
        raise Exception('Failed test: %d\nExpected: %s\nGot: %s' % (number, expected_errors, errors))

    fixed = fix_fan_temperature_sources(fan_temperature_sources, fan_count, available_sensors)

    if fixed != expected_fixed:
        raise Exception('Failed test: %d\nExpected: %s\nGot: %s' % (number, expected_fixed, fixed))

test(
    0,
    [ {} ], 1, AVAILABLE_SENSORS,
    ['FanTemperatureSources[0]: Missing field: FanIndex'],
    []
)

test(
    1,
    [ {'FanIndex': -1} ], 1, AVAILABLE_SENSORS,
    ['FanTemperatureSources[0]: FanIndex: Cannot be negative'],
    []
)

test(
    2,
    [ {'FanIndex': 1} ], 1, AVAILABLE_SENSORS,
    ['FanTemperatureSources[0]: FanIndex: No fan found for FanIndex `1`'],
    []
)

test(
    3,
    [ {'FanIndex': 0, 'TemperatureAlgorithmType': 'Foo'} ], 1, AVAILABLE_SENSORS,
    ['FanTemperatureSources[0]: TemperatureAlgorithmType: Invalid value'],
    [ {'FanIndex': 0} ]
)

test(
    4,
    [ {'FanIndex': 0, 'Sensors': ['Foo']} ], 1, AVAILABLE_SENSORS,
    ['FanTemperatureSources[0]: Sensors[0]: Foo: Sensor not found'],
    [ {'FanIndex': 0} ]
)

test(
    5,
    [ {'FanIndex': 0, 'Foo': 'Bar'} ], 1, AVAILABLE_SENSORS,
    ['FanTemperatureSources[0]: Invalid field: Foo'],
    [ {'FanIndex': 0} ]
)

test(
    6,
    [ {'FanIndex': 0, 'Sensors': ['coretemp']} ], 1, AVAILABLE_SENSORS,
    [],
    [ {'FanIndex': 0, 'Sensors': ['coretemp']} ]
)

test(
    7,
    [ {'FanIndex': 0, 'Sensors': ['/sys/class/hwmon/hwmon1/temp1_input']} ], 1, AVAILABLE_SENSORS,
    [],
    [ {'FanIndex': 0, 'Sensors': ['/sys/class/hwmon/hwmon1/temp1_input']} ]
)
