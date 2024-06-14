/* Auto generated code ['./tools/config.py', 'header'] */

struct TemperatureThreshold {
	short           UpThreshold;
	short           DownThreshold;
	float           FanSpeed;
};

typedef struct TemperatureThreshold TemperatureThreshold;
declare_array_of(TemperatureThreshold);
Error* TemperatureThreshold_FromJson(TemperatureThreshold*, const nx_json*);
Error* TemperatureThreshold_ValidateFields(TemperatureThreshold*);

struct FanSpeedPercentageOverride {
	float           FanSpeedPercentage;
	int             FanSpeedValue;
	OverrideTargetOperation TargetOperation;
};

typedef struct FanSpeedPercentageOverride FanSpeedPercentageOverride;
declare_array_of(FanSpeedPercentageOverride);
Error* FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride*, const nx_json*);
Error* FanSpeedPercentageOverride_ValidateFields(FanSpeedPercentageOverride*);

struct RegisterWriteConfiguration {
	RegisterWriteMode WriteMode;
	RegisterWriteOccasion WriteOccasion;
	short           Register;
	int             Value;
	Boolean         ResetRequired;
	int             ResetValue;
	RegisterWriteMode ResetWriteMode;
	const char*     Description;
};

typedef struct RegisterWriteConfiguration RegisterWriteConfiguration;
declare_array_of(RegisterWriteConfiguration);
Error* RegisterWriteConfiguration_FromJson(RegisterWriteConfiguration*, const nx_json*);
Error* RegisterWriteConfiguration_ValidateFields(RegisterWriteConfiguration*);

struct FanConfiguration {
	const char*     FanDisplayName;
	short           ReadRegister;
	short           WriteRegister;
	int             MinSpeedValue;
	int             MaxSpeedValue;
	int             MinSpeedValueRead;
	int             MaxSpeedValueRead;
	Boolean         IndependentReadMinMaxValues;
	Boolean         ResetRequired;
	int             FanSpeedResetValue;
	array_of(TemperatureThreshold) TemperatureThresholds;
	array_of(FanSpeedPercentageOverride) FanSpeedPercentageOverrides;
};

typedef struct FanConfiguration FanConfiguration;
declare_array_of(FanConfiguration);
Error* FanConfiguration_FromJson(FanConfiguration*, const nx_json*);
Error* FanConfiguration_ValidateFields(FanConfiguration*);

struct ModelConfig {
	const char*     NotebookModel;
	const char*     Author;
	Boolean         LegacyTemperatureThresholdsBehaviour;
	short           EcPollInterval;
	short           CriticalTemperature;
	short           CriticalTemperatureOffset;
	Boolean         ReadWriteWords;
	array_of(FanConfiguration) FanConfigurations;
	array_of(RegisterWriteConfiguration) RegisterWriteConfigurations;
};

typedef struct ModelConfig ModelConfig;
declare_array_of(ModelConfig);
Error* ModelConfig_FromJson(ModelConfig*, const nx_json*);
Error* ModelConfig_ValidateFields(ModelConfig*);

struct FanTemperatureSourceConfig {
	int             FanIndex;
	TemperatureAlgorithmType TemperatureAlgorithmType;
	array_of(str)   Sensors;
};

typedef struct FanTemperatureSourceConfig FanTemperatureSourceConfig;
declare_array_of(FanTemperatureSourceConfig);
Error* FanTemperatureSourceConfig_FromJson(FanTemperatureSourceConfig*, const nx_json*);
Error* FanTemperatureSourceConfig_ValidateFields(FanTemperatureSourceConfig*);

struct ServiceConfig {
	const char*     SelectedConfigId;
	EmbeddedControllerType EmbeddedControllerType;
	array_of(float) TargetFanSpeeds;
	array_of(FanTemperatureSourceConfig) FanTemperatureSources;
};

typedef struct ServiceConfig ServiceConfig;
declare_array_of(ServiceConfig);
Error* ServiceConfig_FromJson(ServiceConfig*, const nx_json*);
Error* ServiceConfig_ValidateFields(ServiceConfig*);

struct FanInfo {
	const char*     name;
	float           temperature;
	Boolean         automode;
	Boolean         critical;
	float           current_speed;
	float           target_speed;
	int             speed_steps;
};

typedef struct FanInfo FanInfo;
declare_array_of(FanInfo);
Error* FanInfo_FromJson(FanInfo*, const nx_json*);
Error* FanInfo_ValidateFields(FanInfo*);

struct ServiceInfo {
	int             pid;
	const char*     config;
	Boolean         read_only;
	array_of(FanInfo) fans;
};

typedef struct ServiceInfo ServiceInfo;
declare_array_of(ServiceInfo);
Error* ServiceInfo_FromJson(ServiceInfo*, const nx_json*);
Error* ServiceInfo_ValidateFields(ServiceInfo*);

