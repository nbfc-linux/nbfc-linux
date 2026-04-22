# NAME

nbfc_service.json - Notebook FanControl service configuration

# CONFIGURATION

**NOTE**: Comments can be added in any JSON object by using the
**Comment** or the **\#** field.

## ServiceConfig

Main configuration file of nbfc_service (*/etc/nbfc/nbfc.json*).

**SelectedConfigId**: *String*

> The selected config model. This equals the basename of a notebook
> model configuration file. Absolute file paths can also be specified.

**EmbeddedControllerType**: *String*

> Selects the implementation used to access the embedded controller.
>
> > -   **ec_sys**: Use the ec_sys kernel module.
> >
> > -   **acpi_ec**: Use the acpi_ec kernel module.
> >
> > -   **dev_port**: Access the embedded controller via /dev/port.
> >
> > -   **dummy**: Do not access the embedded controller at all.
>
> If not given, the embedded controller type will be automatically
> selected.

**FanTemperatureSources**: *Array of FanTemperatureSourceConfig*

> Used for configuring which sensors are attached to a fan.

## FanTemperatureSourceConfig

Configures which sensors are attached to a fan.

**FanIndex**: *Integer* \>= 0 && *Integer* \<= 255

> Specify which fan should be configured.

**TemperatureAlgorithmType**: *String*

> See **FanConfiguration**-\>**TemperatureAlgorithmType**.

**Sensors**: *Array of String*

> See **FanConfiguration**-\>**Sensors**.

## ServiceState

State file of the service (*/var/run/nbfc/state.json*).

**TargetFanSpeeds**: *Array of Float*

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
> temperature drops below (**CriticalTemperature** minus
> **CriticalTemperatureOffset**).

**CriticalTemperatureOffset**: *Integer* \> 0

> See **CriticalTemperature**.

**ReadWriteWords**: *Boolean*

> If **true**, NBFC will combine two 8 bit registers to one 16-bit
> register when reading from or writing to the EC registers.

**FanConfigurations**: *Array of FanConfiguration*

> Array of at least one FanConfiguration

**RegisterWriteConfigurations**: *Array of RegisterWriteConfiguration*

> Array of RegisterWriteConfiguration

## FanConfiguration

Defines how NBFC controls a fan.

**FanDisplayName**: *String*

> Sets the fan display name.

**ReadRegister**: *Integer* \>= 0 && *Integer* \<= 255

> The register from which NBFC reads the fan speed.
>
> This option is mutually exclusive to **ReadAcpiMethod** and
> **ReadLuaCode**. Only one of them can be set at a time.

**ReadAcpiMethod**: *String*

> The ACPI method for reading the fan speed.
>
> Example:
>
> > \"ReadAcpiMethod\": \"\\\\\_SB.PCI0.LPCB.EC0.GFSD\"
>
> This option is mutually exclusive to **ReadRegister** and
> **ReadLuaCode**. Only one of them can be set at a time.

**ReadLuaCode**: *String* or *Array of String*

> The Lua code to be executed for reading the fan speed. See** LUA
> CODE**.
>
> Example:
>
> > \"ReadLuaCode\": \"return
> > acpi_call(\\\"\\\\\\\\\_SB.PCI0.LPCB.EC0.GFSD\\\")\"
>
> This option is mutually exclusive to **ReadRegister** and
> **ReadAcpiMethod**. Only one of them can be set at a time.

**WriteRegister**: *Integer* \>= 0 && *Integer* \<= 255

> The register which NBFC uses to control the fan.
>
> This option is mutually exclusive to **WriteAcpiMethod** and
> **WriteLuaCode**. Only one of them can be set at a time.

**WriteAcpiMethod**: *String*

> The ACPI method for writing the fan speed. The command must contain a
> placeholder (\$) which will be replaced by the fan speed.
>
> Example:
>
> > \"WriteAcpiMethod\": \"\\\\\_SB.PCI0.LPCB.EC0.SFSD \$\"
>
> This option is mutually exclusive to **WriteRegister** and
> **WriteLuaCode**. Only one of them can be set at a time.

**WriteLuaCode**: *String* or *Array of String*

> The Lua code to execute for writing the fan speed. See** LUA CODE**.
>
> Example:
>
> > \"WriteLuaCode\": \"return
> > acpi_call(\\\"\\\\\\\\\_SB.PCI0.LPCB.EC0.SFSD \\\" .. get_value())\"
>
> This option is mutually exclusive to **WriteRegister** and
> **WriteAcpiMethod**. Only one of them can be set at a time.

**MinSpeedValue**: *Integer* \>= 0 && *Integer* \<= 65535

> The value which puts the fan to the lowest possible speed (usually
> this stops the fan). Must be an integer between 0 and 255 or 0 and
> 65535 if **ReadWriteWords** is **true**. Note: **MinSpeedValue** does
> not necessarily have to be smaller than **MaxSpeedValue**.

**MaxSpeedValue**: *Integer* \>= 0 && *Integer* \<= 65535

> The value which puts the fan to the highest possible fan speed.

**MinSpeedValueRead**: *Integer* \>= 0 && *Integer* \<= 65535

> The value which corresponds to the lowest possible fan speed. Will be
> ignored if **IndependentReadMinMaxValues** is **false**.

**MaxSpeedValueRead**: *Integer* \>= 0 && *Integer* \<= 65535

> The value which corresponds to the highest possible fan speed. Will be
> ignored if **IndependentReadMinMaxValues** is **false**.

**IndependentReadMinMaxValues**: *Boolean*

> Defines if independent minimum/maximum values should be applied for
> read operations.
>
> This enables **MinSpeedValueRead** and **MaxSpeedValueRead**.

**ResetRequired**: *Boolean*

> Defines if the EC should be reset before the service is shut down.

**FanSpeedResetValue**: *Integer* \>= 0 && *Integer* \<= 65535

> Defines the value which will be written to **WriteRegister** to reset
> the EC.
>
> This option is mutually exclusive to **ResetAcpiMethod** and
> **ResetLuaCode**. Only one of them can be set at a time.

**ResetAcpiMethod**: *String*

> The ACPI method to call upon fan reset.
>
> This option is mutually exclusive to **FanSpeedResetValue** and
> **ResetLuaCode**. Only one of them can be set at a time.

**ResetLuaCode**: *String* or *Array of String*

> The Lua code to be executed upon fan reset. See** LUA CODE**.
>
> This option is mutually exclusive to **FanSpeedResetValue** and
> **ResetAcpiMethod**. Only one of them can be set at a time.

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

**TemperatureThresholds**: *Array of TemperatureThreshold*

> Defines how fast the fan runs at different temperatures.

**FanSpeedPercentageOverrides**: *Array of FanSpeedPercentageOverride*

> Overrides the default algorithm to calculate fan speeds.

## RegisterWriteConfiguration

Allows to write to any EC register.

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
> -   **Call**: calls the ACPI method stored in **AcpiMethod** or
>     **ResetAcpiMethod**
>
> -   **Lua**: executes the Lua code stored in **LuaCode** or
>     **ResetLuaCode**

**WriteOccasion**: *String*

> Defines when the value should be written:
>
> -   **OnInitialization**: writes the value once upon initialization
>     (everytime the fan control service starts).
>
> -   **OnWriteFanSpeed**: writes the value everytime the fan speed is
>     set.

**Register**: *Integer* \>= 0 && *Integer* \<= 255

> The register which will be manipulated.

**Value**: *Integer* \>= 0 && *Integer* \<= 255

> The value which will be written.
>
> This option is mutually exclusive to **AcpiMethod** and **LuaCode**.
> Only one of them can be set at a time.

**AcpiMethod**: *String*

> The ACPI method to call.
>
> This option is mutually exclusive to **Value** and **LuaCode**. Only
> one of them can be set at a time.

**LuaCode**: *String* or *Array of String*

> The Lua code to be executed. See** LUA CODE**.
>
> This option is mutually exclusive to **Value** and **AcpiMethod**.
> Only one of them can be set at a time.

**ResetRequired**: *Boolean*

> Defines if the register should be reset before the service is shut
> down.

**ResetValue**: *Integer* \>= 0 && *Integer* \<= 255

> The value which will be written upon reset.
>
> This option is mutually exclusive to **ResetAcpiMethod** and
> **ResetLuaCode**. Only one of them can be set at a time.

**ResetAcpiMethod**: *String*

> The ACPI method to call upon reset.
>
> This option is mutually exclusive to **ResetValue** and
> **ResetLuaCode**. Only one of them can be set at a time.

**ResetLuaCode**: *String* or *Array of String*

> The Lua code to be executed upon reset. See** LUA CODE**.
>
> This option is mutually exclusive to **ResetValue** and
> **ResetAcpiMethod**. Only one of them can be set at a a time.

**ResetWriteMode**: *String*

> See **WriteMode**. Will only be applied on reset.

**Description**: *String*

> A short description of what effect the **RegisterWriteConfiguration**
> will have.

## FanSpeedPercentageOverride

Overrides the default algorithm to calculate fan speeds.

**FanSpeedPercentage**: *Float* \>= 0.0 && *Float* \<= 100.0

> The fan speed in percent.

**FanSpeedValue**: *Integer* \>= 0 && *Integer* \<= 65535

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
> exceeds *UpThreshold* (in celsius).

**DownThreshold**: *Integer*

> NBFC will select the next lower threshold as soon as the temperature
> falls below the *DownThreshold* (in celsius).

**FanSpeed**: *Float* \>= 0.0 && *Float* \<= 100.0

> The fan speed in percent.

# LUA CODE

NBFC-Linux allows executing **Lua** code to implement more complex read
or write operations.

Lua code can be specified as either a *String* or an *Array of String*.

## Return Values

Every Lua code must return exactly to values: **Error**, **Result.**

**Error** is either **nil** (on success) or a **string** describing the
error (on failure).

**Result** is an **integer** and is used for read operations. For write
operations this value is ignored (returning **0** is recommended).

## Availabe Lua Functions

The following functions are exposed to Lua:

**acpi_call**(**method**):

> Executes an ACPI method. Returns **error**, **value**.

**ec_read**(**register**):

> Reads a byte from the embedded controller. Returns **error**,
> **value**.

**ec_read_word**(**register**):

> Reads two bytes and combines them into a single value. Returns
> **error**, **value**.

**ec_write**(**register**, **value**):

> Writes a byte to the embedded controller. Returns **error**, **0**.

**ec_write_word**(**register**, **value**):

> Writes a 16-bit value to a two-byte register. Returns **error**,
> **0**.

# FILES

*/etc/nbfc/nbfc.json*

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
