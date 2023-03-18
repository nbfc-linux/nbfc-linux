NBFC\_SERVICE 5 "MARCH 2021" Notebook FanControl
================================================

NAME
----

nbfc\_service.json - Notebook FanControl service configuration

ServiceConfig
-------------

Main configuration file of nbfc_service

`"SelectedConfigId"`: *const char\**
  The selected config model. This equals the basename of a notebook model configuration file.

`"ReadOnly"`: *Boolean*
  If `true`, fan speeds will not be written to the embedded controller

`"EmbeddedControllerType"`: *EmbeddedControllerType*
  Either `ECSysLinux` for using the `ec_sys` kernel module, or `ECLinux` for an alternative implementation using `/dev/port` without depending on kernel modules.

`"TargetFanSpeeds"`: *array_of(float)*
  This holds the fixed speed of the fans. A value of `-1` means the fan should be left in auto mode.



ModelConfig
-----------



`"NotebookModel"`: *const char\**
  The Notebook Model as described in your BIOS. See `sudo dmidecode -s system-product-name`.

`"Author"`: *const char\**
  The Author of the config file. Enter whatever you want.

`"EcPollInterval"`: *short > 0*
  Defines how often NBFC polls the EC for changes (in miliseconds).

`"CriticalTemperature"`: *short*
  If the temperature exceeds this threshold, NBFC will ignore all Temperature threshold elements and set the fan to 100% speed until the temperature drops below (`CriticalTemperature` - `CriticalTemperatureOffset`)

`"CriticalTemperatureOffset"`: *short > 0*
  See `CriticalTemperature`

`"ReadWriteWords"`: *Boolean*
  If `true`, NBFC will combine two 8 bit registers to one 16-bit register when reading from or writing to the EC registers.

`"FanConfigurations"`: *array_of(FanConfiguration).size > 0*
  Array of at least one FanConfiguration

`"RegisterWriteConfigurations"`: *array_of(RegisterWriteConfiguration)*
  Array of RegisterWriteConfiguration



FanConfiguration
----------------

Defines how NBFC controls a fan

`"FanDisplayName"`: *const char\**
  Fan display name

`"ReadRegister"`: *short >= 0 && short <= 255*
  The register from which NBFC reads the fan speed.

`"WriteRegister"`: *short >= 0 && short <= 255*
  The register which NBFC uses to control the fan

`"MinSpeedValue"`: *int >= 0 && int <= 65535*
  The value which puts the fan to the lowest possible speed (usually this stops the fan). Must be an integer between 0 and 255 or 0 and 65535 if ReadWriteWords is `true`. Note: MinSpeedValue does not necessarily have to be smaller than MaxSpeedValue

`"MaxSpeedValue"`: *int >= 0 && int <= 65535*
  The value which puts the fan to the highest possible fan speed.

`"MinSpeedValueRead"`: *int >= 0 && int <= 65535*
  The value which corresponds to the lowest possible fan speed. Will be ignored if IndependentReadMinMaxValues is `false`.

`"MaxSpeedValueRead"`: *int >= 0 && int <= 65535*
  The value which  corresponds to the highest possible fan speed. Will be ignored if IndependentReadMinMaxValues is `false`.

`"IndependentReadMinMaxValues"`: *Boolean*
  Defines if independent minimum/maximum values should be applied for read operations

`"ResetRequired"`: *Boolean*
  Defines if the EC should be reset before the service is shut down.

`"FanSpeedResetValue"`: *int >= 0 && int <= 65535*
  Defines the value which will be written to WriteRegister to reset the EC.

`"TemperatureThresholds"`: *array_of(TemperatureThreshold)*
  

`"FanSpeedPercentageOverrides"`: *array_of(FanSpeedPercentageOverride)*
  



RegisterWriteConfiguration
--------------------------

Allows to write to any EC register

`"WriteMode"`: *RegisterWriteMode*
  Defines how the value will be written:
  
  - And: register = register bitwise_and value
  - Or: register = register bitwise_or value
  - Set register = value
  

`"WriteOccasion"`: *RegisterWriteOccasion*
  Defines when the value should be written:
  
  - OnInitialization: writes the value once upon initialization (everytimee the fan control service is enabled or a config is applied)
  - OnWriteFanSpeed: writes the value everytimee the fan speed is set.
  

`"Register"`: *short >= 0 && short <= 255*
  The register which will be manipulated.

`"Value"`: *int >= 0 && int <= 65535*
  The Value which will be written

`"ResetRequired"`: *Boolean*
  Defines if the register should be reset before the service is shut down.

`"ResetValue"`: *int >= 0 && int <= 65535*
  The value which will be written upon reset.

`"ResetWriteMode"`: *RegisterWriteMode*
  See WriteMode. Will only be applied on reset. Valid values: Set/And/Or

`"Description"`: *const char\**
  A short description of what effect the RegisterWriteConfiguration will have



FanSpeedPercentageOverride
--------------------------

Overrides the default algorithm to calculate fan speeds

`"FanSpeedPercentage"`: *float >= 0.0 && float <= 100.0*
  The fan speed in percent

`"FanSpeedValue"`: *int >= 0 && int <= 65535*
  Fan fan speed value which will be written to WriteRegister

`"TargetOperation"`: *OverrideTargetOperation*
  Defines for which operations the speeds should be overridden:
  
  - Read: Value->Percentage
  - Write: Percentage->Value
  - ReadWrite: both



TemperatureThreshold
--------------------

Defines how fast the fan runs at different temperatures

`"UpThreshold"`: *short*
  NBFC will select the next upper threshold as soon as the temperature exceeds *UpThreshold*. in celsius

`"DownThreshold"`: *short*
  NBFC will select the next lower threshold as soon as the temperature falls below the *DownThreshold*. in celsius

`"FanSpeed"`: *float >= 0.0 && float <= 100.0*
  The fan speed in percent



FILES
-----

*/var/run/nbfc_service.state.json*
  State file of nbfc\_service. Updated every *EcPollInterval* miliseconds.

*/etc/nbfc/nbfc.json*
  The system wide configuration file.

*/usr/share/nbfc/configs/\*.json*
  Configuration files for various notebook models.

AUTHOR
------

Benjamin Abendroth (braph93@gmx.de)

SEE ALSO
--------

[nbfc(1)](nbfc.md), [nbfc\_service(1)](nbfc_service.md), [ec\_probe(1)](ec_probe.md), fancontrol(1), sensors(1)

