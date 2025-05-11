Advanced configuration
----------------------

> NOTE: Since version 0.3.3 NBFC-Linux also allows you to specify sensors in FanConfigurations using the `Sensors` field

**Default Configuration**

If no configuration is specified, NBFC uses the "Average" algorithm and utilizes all sensor files named "coretemp", "k10temp", or "zenpower".

**Available Algorithms**

You can choose from three different algorithms to compute the temperature:

- *"Average"*: Computes the average temperature from all specified sources.
- *"Min"*: Selects the lowest temperature among all specified sources.
- *"Max"*: Selects the highest temperature among all specified sources.

**Specifying Temperature Sources**

You can specify temperature sources either by a
- *sensor name* (which may result in multiple temperature sources)
- *temperature file*: A file containing the temperature (usually named `temp*_input`)
- *shell command*: A command whose output is used as temperature (the command has to be prefixed by `$`)
- *sensor group*:
  - *@CPU*: Uses all sensors named "coretemp", "k10temp" or "zenpower"
  - *@GPU*: Uses all sensors named "amdgpu", "nvidia", "nouveau" or "radeon"

**Example Configuration**

Here is a fictional example demonstrating how to configure NBFC-Linux:

```
{
    "SelectedConfigId": "Asus G53SX",
    "TargetFanSpeeds": [ -1.000000 ],
    "FanTemperatureSources": [
        {
            "FanIndex": 0,
            "TemperatureAlgorithmType": "Min",
            "Sensors": [ "coretemp" ]
        },
        {
            "FanIndex": 1,
            "TemperatureAlgorithmType": "Average",
            "Sensors": [ "nouveau" ]
        },
        {
            "FanIndex": 2,
            "TemperatureAlgorithmType": "Average",
            "Sensors": [ "/sys/class/hwmon/hwmon4/temp2_input", "/sys/class/hwmon/hwmon4/temp3_input" ]
        },
        {
            "FanIndex": 3,
            "Sensors": [ "$ echo 42" ]
        },
        {
            "FanIndex": 4,
            "Sensors": [ "@GPU" ]
        }
    ]
}
```

In this example:

- *Fan 0* uses the "Min" algorithm with sensors named "coretemp".
- *Fan 1* uses the "Average" algorithm with sensors named "nouveau".
- *Fan 2* uses the default algorithm with specific sensor file paths.
- *Fan 3* uses the output of `echo 42` as temperature
- *Fan 4* uses all sensors found in the `@GPU` group ("amdgpu", "nvidia", "nouveau" or "radeon")
