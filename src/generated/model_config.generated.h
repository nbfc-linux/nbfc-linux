/* Auto generated code ['./tools/config.py', 'header'] */

struct TemperatureThreshold {
	int16_t         UpThreshold;
	int16_t         DownThreshold;
	float           FanSpeed;
	struct {
		bool UpThreshold     : 1;
		bool DownThreshold   : 1;
		bool FanSpeed        : 1;
	} isset;
};

typedef struct TemperatureThreshold TemperatureThreshold;
declare_array_of(TemperatureThreshold);
Error TemperatureThreshold_FromJson(TemperatureThreshold*, const nx_json*);
Error TemperatureThreshold_ValidateFields(TemperatureThreshold*);

struct FanSpeedPercentageOverride {
	float           FanSpeedPercentage;
	uint16_t        FanSpeedValue;
	OverrideTargetOperation TargetOperation;
	struct {
		bool FanSpeedPercentage : 1;
		bool FanSpeedValue   : 1;
		bool TargetOperation : 1;
	} isset;
};

typedef struct FanSpeedPercentageOverride FanSpeedPercentageOverride;
declare_array_of(FanSpeedPercentageOverride);
Error FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride*, const nx_json*);
Error FanSpeedPercentageOverride_ValidateFields(FanSpeedPercentageOverride*);

struct RegisterWriteConfiguration {
	RegisterWriteMode WriteMode;
	RegisterWriteOccasion WriteOccasion;
	uint8_t         Register;
	uint8_t         Value;
	const char*     AcpiMethod;
	LuaCode         LuaCode;
	bool            ResetRequired;
	uint8_t         ResetValue;
	const char*     ResetAcpiMethod;
	LuaCode         ResetLuaCode;
	RegisterWriteMode ResetWriteMode;
	const char*     Description;
	struct {
		bool WriteMode       : 1;
		bool WriteOccasion   : 1;
		bool Register        : 1;
		bool Value           : 1;
		bool AcpiMethod      : 1;
		bool LuaCode         : 1;
		bool ResetRequired   : 1;
		bool ResetValue      : 1;
		bool ResetAcpiMethod : 1;
		bool ResetLuaCode    : 1;
		bool ResetWriteMode  : 1;
		bool Description     : 1;
	} isset;
};

typedef struct RegisterWriteConfiguration RegisterWriteConfiguration;
declare_array_of(RegisterWriteConfiguration);
Error RegisterWriteConfiguration_FromJson(RegisterWriteConfiguration*, const nx_json*);
Error RegisterWriteConfiguration_ValidateFields(RegisterWriteConfiguration*);

struct FanConfiguration {
	const char*     FanDisplayName;
	uint8_t         ReadRegister;
	const char*     ReadAcpiMethod;
	LuaCode         ReadLuaCode;
	uint8_t         WriteRegister;
	const char*     WriteAcpiMethod;
	LuaCode         WriteLuaCode;
	uint16_t        MinSpeedValue;
	uint16_t        MaxSpeedValue;
	uint16_t        MinSpeedValueRead;
	uint16_t        MaxSpeedValueRead;
	bool            IndependentReadMinMaxValues;
	bool            ResetRequired;
	uint16_t        FanSpeedResetValue;
	const char*     ResetAcpiMethod;
	LuaCode         ResetLuaCode;
	TemperatureAlgorithmType TemperatureAlgorithmType;
	array_of(str)   Sensors;
	array_of(TemperatureThreshold) TemperatureThresholds;
	array_of(FanSpeedPercentageOverride) FanSpeedPercentageOverrides;
	struct {
		bool FanDisplayName  : 1;
		bool ReadRegister    : 1;
		bool ReadAcpiMethod  : 1;
		bool ReadLuaCode     : 1;
		bool WriteRegister   : 1;
		bool WriteAcpiMethod : 1;
		bool WriteLuaCode    : 1;
		bool MinSpeedValue   : 1;
		bool MaxSpeedValue   : 1;
		bool MinSpeedValueRead : 1;
		bool MaxSpeedValueRead : 1;
		bool IndependentReadMinMaxValues : 1;
		bool ResetRequired   : 1;
		bool FanSpeedResetValue : 1;
		bool ResetAcpiMethod : 1;
		bool ResetLuaCode    : 1;
		bool TemperatureAlgorithmType : 1;
		bool Sensors         : 1;
		bool TemperatureThresholds : 1;
		bool FanSpeedPercentageOverrides : 1;
	} isset;
};

typedef struct FanConfiguration FanConfiguration;
declare_array_of(FanConfiguration);
Error FanConfiguration_FromJson(FanConfiguration*, const nx_json*);
Error FanConfiguration_ValidateFields(FanConfiguration*);

struct Sponsor {
	const char*     Name;
	const char*     Description;
	const char*     URL;
	const char*     BannerURL;
	struct {
		bool Name            : 1;
		bool Description     : 1;
		bool URL             : 1;
		bool BannerURL       : 1;
	} isset;
};

typedef struct Sponsor Sponsor;
declare_array_of(Sponsor);
Error Sponsor_FromJson(Sponsor*, const nx_json*);
Error Sponsor_ValidateFields(Sponsor*);

struct ModelConfig {
	const char*     NotebookModel;
	const char*     Author;
	bool            LegacyTemperatureThresholdsBehaviour;
	bool            LuaLibraries;
	uint16_t        EcPollInterval;
	int16_t         CriticalTemperature;
	uint16_t        CriticalTemperatureOffset;
	bool            ReadWriteWords;
	Sponsor         Sponsor;
	array_of(str)   FirmwareFingerprint;
	array_of(FanConfiguration) FanConfigurations;
	array_of(RegisterWriteConfiguration) RegisterWriteConfigurations;
	struct {
		bool NotebookModel   : 1;
		bool Author          : 1;
		bool LegacyTemperatureThresholdsBehaviour : 1;
		bool LuaLibraries    : 1;
		bool EcPollInterval  : 1;
		bool CriticalTemperature : 1;
		bool CriticalTemperatureOffset : 1;
		bool ReadWriteWords  : 1;
		bool Sponsor         : 1;
		bool FirmwareFingerprint : 1;
		bool FanConfigurations : 1;
		bool RegisterWriteConfigurations : 1;
	} isset;
};

typedef struct ModelConfig ModelConfig;
declare_array_of(ModelConfig);
Error ModelConfig_FromJson(ModelConfig*, const nx_json*);
Error ModelConfig_ValidateFields(ModelConfig*);

struct FanTemperatureSourceConfig {
	uint8_t         FanIndex;
	TemperatureAlgorithmType TemperatureAlgorithmType;
	array_of(str)   Sensors;
	struct {
		bool FanIndex        : 1;
		bool TemperatureAlgorithmType : 1;
		bool Sensors         : 1;
	} isset;
};

typedef struct FanTemperatureSourceConfig FanTemperatureSourceConfig;
declare_array_of(FanTemperatureSourceConfig);
Error FanTemperatureSourceConfig_FromJson(FanTemperatureSourceConfig*, const nx_json*);
Error FanTemperatureSourceConfig_ValidateFields(FanTemperatureSourceConfig*);

struct ServiceConfig {
	const char*     SelectedConfigId;
	EmbeddedControllerType EmbeddedControllerType;
	array_of(float) TargetFanSpeeds;
	array_of(FanTemperatureSourceConfig) FanTemperatureSources;
	struct {
		bool SelectedConfigId : 1;
		bool EmbeddedControllerType : 1;
		bool TargetFanSpeeds : 1;
		bool FanTemperatureSources : 1;
	} isset;
};

typedef struct ServiceConfig ServiceConfig;
declare_array_of(ServiceConfig);
Error ServiceConfig_FromJson(ServiceConfig*, const nx_json*);
Error ServiceConfig_ValidateFields(ServiceConfig*);

struct ServiceState {
	array_of(float) TargetFanSpeeds;
	struct {
		bool TargetFanSpeeds : 1;
	} isset;
};

typedef struct ServiceState ServiceState;
declare_array_of(ServiceState);
Error ServiceState_FromJson(ServiceState*, const nx_json*);
Error ServiceState_ValidateFields(ServiceState*);

struct FanInfo {
	const char*     Name;
	float           Temperature;
	bool            AutoMode;
	bool            Critical;
	float           CurrentSpeed;
	float           TargetSpeed;
	float           RequestedSpeed;
	uint16_t        SpeedSteps;
	struct {
		bool Name            : 1;
		bool Temperature     : 1;
		bool AutoMode        : 1;
		bool Critical        : 1;
		bool CurrentSpeed    : 1;
		bool TargetSpeed     : 1;
		bool RequestedSpeed  : 1;
		bool SpeedSteps      : 1;
	} isset;
};

typedef struct FanInfo FanInfo;
declare_array_of(FanInfo);
Error FanInfo_FromJson(FanInfo*, const nx_json*);
Error FanInfo_ValidateFields(FanInfo*);

struct ServiceInfo {
	int             PID;
	const char*     SelectedConfigId;
	bool            ReadOnly;
	array_of(FanInfo) Fans;
	struct {
		bool PID             : 1;
		bool SelectedConfigId : 1;
		bool ReadOnly        : 1;
		bool Fans            : 1;
	} isset;
};

typedef struct ServiceInfo ServiceInfo;
declare_array_of(ServiceInfo);
Error ServiceInfo_FromJson(ServiceInfo*, const nx_json*);
Error ServiceInfo_ValidateFields(ServiceInfo*);

