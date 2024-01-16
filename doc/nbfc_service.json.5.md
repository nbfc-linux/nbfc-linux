# NAME

nbfc_service.json - Notebook FanControl service configuration

# CONFIGURATION

## ServiceConfig

Main configuration file of nbfc_service (*/etc/nbfc.json*)

**SelectedConfigId**: *String*

> The selected config model. This equals the basename of a notebook
> model configuration file.
>
> **EmbeddedControllerType**: *String*
>
> > -   **ec_sys_linux**: Use the ec_sys kernel module for writing to
> >     the embedded controller.
> >
> > -   **ec_acpi**: Use the acpi_ec kernel module for writing to the
> >     embedded controller.
> >
> > -   **ec_linux**: Write to the embedded controller using /dev/port.
> >
> > -   **dummy**: Don\'t write to the embedded controller at all.
>
> If not given, the embedded controller type will be automatically
> selected.

**TargetFanSpeeds**: *Array of Floats*

> This holds the fixed speed of the fans. A value of **-1** means the
> fan should be left in auto mode.

## ModelConfig

**NotebookModel**: *String*

> The Notebook Model as described in your BIOS. See **nbfc
> get-model-name.**

**Author**: *String*

> The Author of the config file. Enter whatever you want.

**EcPollInterval**: *Integer* \> 0

> Defines how often NBFC polls the EC for changes (in milliseconds).

**CriticalTemperature**: *Integer*

> If the temperature exceeds this threshold, NBFC will ignore all
> Temperature threshold elements and set the fan to 100% speed until the
> temperature drops below
> (**CriticalTemperature**-**CriticalTemperatureOffset**).

**CriticalTemperatureOffset**: *Integer* \> 0

> See **CriticalTemperature**

**ReadWriteWords**: *Boolean*

> If **true**, NBFC will combine two 8 bit registers to one 16-bit
> register when reading from or writing to the EC registers.

**FanConfigurations**: *Array of FanConfigurations*

> Array of at least one FanConfiguration

**RegisterWriteConfigurations**: *Array of RegisterWriteConfiguration*

> Array of RegisterWriteConfiguration

## FanConfiguration

Defines how NBFC controls a fan

**FanDisplayName**: *String*

> Fan display name

**ReadRegister**: *Integer* \>= 0 && *Integer* \<= 255

> The register from which NBFC reads the fan speed.

**WriteRegister**: *Integer* \>= 0 && *Integer* \<= 255

> The register which NBFC uses to control the fan.

**MinSpeedValue**: *Integer*

> The value which puts the fan to the lowest possible speed (usually
> this stops the fan). Must be an integer between 0 and 255 or 0 and
> 65535 if **ReadWriteWords** is **true**. Note: **MinSpeedValue** does
> not necessarily have to be smaller than **MaxSpeedValue.**

**MaxSpeedValue**: *Integer*

> The value which puts the fan to the highest possible fan speed.

**MinSpeedValueRead**: *Integer*

> The value which corresponds to the lowest possible fan speed. Will be
> ignored if **IndependentReadMinMaxValues** is **false**.

**MaxSpeedValueRead**: *Integer*

> The value which corresponds to the highest possible fan speed. Will be
> ignored if **IndependentReadMinMaxValues** is **false**.

**IndependentReadMinMaxValues**: *Boolean*

> Defines if independent minimum/maximum values should be applied for
> read operations.

**ResetRequired**: *Boolean*

> Defines if the EC should be reset before the service is shut down.

**FanSpeedResetValue**: *Integer*

> Defines the value which will be written to **WriteRegister** to reset
> the EC.

**TemperatureThresholds**: *Array of TemperatureThresholds*

**FanSpeedPercentageOverrides**: *Array of FanSpeedPercentageOverride*

## RegisterWriteConfiguration

Allows to write to any EC register

**WriteMode**: *String*

> Defines how the value will be written:
>
> -   **And**: performs a binary AND operation (register = register &
>     value)
>
> -   **Or**: performs a binary OR operation (register = register \|
>     value)
>
> -   **Set**: overwrites the register with the specified value
>     (register = value)

**WriteOccasion**: *String*

> Defines when the value should be written:
>
> -   **OnInitialization**: writes the value once upon initialization
>     (everytimee the fan control service is enabled or a config is
>     applied)
>
> -   **OnWriteFanSpeed**: writes the value everytimee the fan speed is
>     set.

**Register**: *Integer* \>= 0 && *Integer* \<= 255

> The register which will be manipulated.

**Value**: *Integer*

> The Value which will be written

**ResetRequired**: *Boolean*

> Defines if the register should be reset before the service is shut
> down.

**ResetValue**: *Integer*

> The value which will be written upon reset.

**ResetWriteMode**: *String*

> See **WriteMode**. Will only be applied on reset.

**Description**: *String*

> A short description of what effect the **RegisterWriteConfiguration**
> will have.

## FanSpeedPercentageOverride

Overrides the default algorithm to calculate fan speeds.

**FanSpeedPercentage**: *Float* \>= 0.0 && *Float* \<= 100.0

> The fan speed in percent.

**FanSpeedValue**: *Integer*

> The fan speed value which will be written to **WriteRegister**.

**TargetOperation**: *String*

> Defines for which operations the speeds should be overridden:
>
> > -   **Read**: if the value in the EC\'s ReadRegister is equal to
> >     FanSpeedValue, the corresponding percentage will not be
> >     calculated, but instead be set to FanSpeedPercentage.
> >
> > -   **Write**: if the target fan speed percentage is equal to
> >     FanSpeedPercentage, the corresponding value will not be
> >     calculated, but instead be set to FanSpeedValue.
> >
> > -   **ReadWrite**: applies to both, read and write operations.

## TemperatureThreshold

Defines how fast the fan runs at different temperatures.

**UpThreshold**: *Integer*

> NBFC will select the next upper threshold as soon as the temperature
> exceeds *UpThreshold*(in celsius).

**DownThreshold**: *Integer*

> NBFC will select the next lower threshold as soon as the temperature
> falls below the *DownThreshold*(in celsius).

**FanSpeed**: *Float* \>= 0.0 && *Float* \<= 100.0

> The fan speed in percent.

# FILES

*/var/run/nbfc_service.state.json*

> State file of nbfc_service. Updated every **EcPollInterval**
> milliseconds.

*/etc/nbfc.json*

> The system wide configuration file.

*/usr/share/configs/\*.json*

> Configuration files for various notebook models.

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc**(1), **nbfc_service**(1), **ec_probe**(1), **fancontrol**(1),
**sensors**(1)
