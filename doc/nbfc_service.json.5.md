# NAME

nbfc_service.json - Notebook FanControl service configuration

# CONFIGURATION

**NOTE**: Comments can be added in any JSON object by using the
**Comment** field.

## ServiceConfig

Main configuration file of nbfc_service (*/etc/nbfc.json*)

**SelectedConfigId**: *String*

> The selected config model. This equals the basename of a notebook
> model configuration file. Absolute file paths can also be specified.

**EmbeddedControllerType**: *String*

> > -   **ec_sys**: Use the ec_sys kernel module for writing to the
> >     embedded controller.
> >
> > -   **acpi_ec**: Use the acpi_ec kernel module for writing to the
> >     embedded controller.
> >
> > -   **dev_port**: Write to the embedded controller using /dev/port.
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

**LegacyTemperatureThresholdsBehaviour**: *Boolean*

> This option was introduced for beeing backwards compatible with
> configuration files from the original NBFC project. The default is
> **false**. Do not use this option for new configuration files.

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

**ReadAcpiCommand**: *String*

> The ACPI method for reading the fan speed.
>
> Example:
>
> > \"ReadAcpiCommand\": \"\\\\\_SB.PCI0.LPCB.EC0.GFSD\"
>
> This option is mutually exclusive to **ReadRegister**. Only one of
> them can be set at a time.

**WriteAcpiCommand**: *String*

> The ACPI method for writing the fan speed. The command must contain a
> placeholder (\$) which will be replaced by the fan speed.
>
> Example:
>
> > \"WriteAcpiCommand\": \"\\\\\_SB.PCI0.LPCB.EC0.SFSD \$\"
>
> This option is mutually exclusive to **WriteRegister**. Only one of
> them can be set at a time.

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

**ResetAcpiCommand**: *String*

> The ACPI method to call upon fan reset.
>
> This option is mutually exclusive to **FanSpeedResetValue**. Only one
> of them can be set at a time.

**Sensors**: *Array of String*

> Defines which sensors shall be used for this fan. This should either
> be a sensor name or a sensor group.
>
> Available groups:
>
> -   **\@CPU**: Uses all sensors named **coretemp**, **k10temp** or
>     **zenpower**
>
> -   **\@GPU**: Uses all sensors named **amdgpu**, **nvidia**,
>     **nvidia-ml**, **nouveau** or **radeon**

**TemperatureAlgorithmType**: *String*

> Specify the algorithm type to compute the temperature.
>
> -   **Average**: Computes the average temperature from all specified
>     sensors (default)
>
> -   **Min**: Selects the lowest temperature among all specified
>     sensors
>
> -   **Max**: Selects the highest temperature among all specified
>     sensors

**TemperatureThresholds**: *Array of TemperatureThresholds*

**FanSpeedPercentageOverrides**: *Array of FanSpeedPercentageOverride*

## RegisterWriteConfiguration

Allows to write to any EC register

**WriteMode**: *String*

> Defines how the value will be written:
>
> -   **Set**: overwrites the register with the specified value
>     (register = value)
>
> -   **And**: performs a binary AND operation (register = register &
>     value)
>
> -   **Or**: performs a binary OR operation (register = register \|
>     value)
>
> -   **Call**: calls the ACPI method stored in **AcpiCommand** or
>     **ResetAcpiCommand**

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

> The value which will be written.

**AcpiCommand**: *String*

> The ACPI method to call.
>
> This option is mutually exclusive to **Value**. Only one of them can
> be set at a time.

**ResetRequired**: *Boolean*

> Defines if the register should be reset before the service is shut
> down.

**ResetValue**: *Integer*

> The value which will be written upon reset.

**ResetAcpiCommand**: *String*

> The ACPI method to call upon reset.
>
> This option is mutually exclusive to **ResetValue**. Only one of them
> can be set at a time.

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
> >     **FanSpeedValue**, the corresponding percentage will not be
> >     calculated, but instead be set to **FanSpeedPercentage**.
> >
> > -   **Write**: if the target fan speed percentage is equal to
> >     **FanSpeedPercentage**, the corresponding value will not be
> >     calculated, but instead be set to **FanSpeedValue**.
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

*/etc/nbfc.json*

> The main configuration file for nbfc_service.

*/var/run/nbfc_service.pid*

> File containing the PID of current running nbfc_service.

*/var/run/nbfc_service.socket*

> Socket file of nbfc_service.

*/usr/share/nbfc/configs/\*.json*

> Configuration files for various notebook models.

*/usr/share/nbfc/model_support.json*

> Model support database. This file maps notebook model names to
> configuration files.

*/var/lib/nbfc/configs/\*.json*

> Configuration files downloaded from the internet. Files in this
> directory take precedence over those in */usr/share/nbfc/configs*.

*/var/lib/nbfc/model_support.json*

> Model support database downloaded from the internet. Both
> */var/lib/nbfc/model_support.json* and
> */usr/share/nbfc/model_support.json* are taken into account.

*/var/lib/nbfc/state.json*

> State file of nbfc_service. This holds the current fan speeds.

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc**(1), **nbfc_service**(1), **ec_probe**(1), **fancontrol**(1),
**sensors**(1)
