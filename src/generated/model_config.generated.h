/* Auto generated code ['../tools/config.py', 'header'] */

struct TemperatureThreshold {
	short           UpThreshold;
	short           DownThreshold;
	float           FanSpeed;
};

typedef struct TemperatureThreshold TemperatureThreshold;
declare_array_of(TemperatureThreshold);
Error* TemperatureThreshold_FromJson(TemperatureThreshold*, const nx_json*);

struct FanSpeedPercentageOverride {
	float           FanSpeedPercentage;
	short           FanSpeedValue;
	OverrideTargetOperation TargetOperation;
};

typedef struct FanSpeedPercentageOverride FanSpeedPercentageOverride;
declare_array_of(FanSpeedPercentageOverride);
Error* FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride*, const nx_json*);

struct RegisterWriteConfiguration {
	RegisterWriteMode WriteMode;
	RegisterWriteOccasion WriteOccasion;
	short           Register;
	short           Value;
	Boolean         ResetRequired;
	short           ResetValue;
	RegisterWriteMode ResetWriteMode;
	const char*     Description;
};

typedef struct RegisterWriteConfiguration RegisterWriteConfiguration;
declare_array_of(RegisterWriteConfiguration);
Error* RegisterWriteConfiguration_FromJson(RegisterWriteConfiguration*, const nx_json*);

struct FanConfiguration {
	const char*     FanDisplayName;
	short           ReadRegister;
	short           WriteRegister;
	short           MinSpeedValue;
	short           MaxSpeedValue;
	short           MinSpeedValueRead;
	short           MaxSpeedValueRead;
	Boolean         IndependentReadMinMaxValues;
	Boolean         ResetRequired;
	short           FanSpeedResetValue;
	array_of(TemperatureThreshold) TemperatureThresholds;
	array_of(FanSpeedPercentageOverride) FanSpeedPercentageOverrides;
};

typedef struct FanConfiguration FanConfiguration;
declare_array_of(FanConfiguration);
Error* FanConfiguration_FromJson(FanConfiguration*, const nx_json*);

struct Config {
	const char*     NotebookModel;
	const char*     Author;
	short           EcPollInterval;
	short           CriticalTemperature;
	short           CriticalTemperatureOffset;
	Boolean         ReadWriteWords;
	array_of(FanConfiguration) FanConfigurations;
	array_of(RegisterWriteConfiguration) RegisterWriteConfigurations;
};

typedef struct Config Config;
declare_array_of(Config);
Error* Config_FromJson(Config*, const nx_json*);

struct ServiceConfig {
	const char*     SelectedConfigId;
	Boolean         ReadOnly;
	EmbeddedControllerType EmbeddedControllerType;
	array_of(float) TargetFanSpeeds;
};

typedef struct ServiceConfig ServiceConfig;
declare_array_of(ServiceConfig);
Error* ServiceConfig_FromJson(ServiceConfig*, const nx_json*);

