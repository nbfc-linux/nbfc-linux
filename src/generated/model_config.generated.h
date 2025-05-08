/* Auto generated code ['./tools/config.py', 'header'] */

struct TemperatureThreshold {
	int16_t         UpThreshold;
	int16_t         DownThreshold;
	float           FanSpeed;
	uint8_t         _set;
};

typedef struct TemperatureThreshold TemperatureThreshold;
declare_array_of(TemperatureThreshold);
Error* TemperatureThreshold_FromJson(TemperatureThreshold*, const nx_json*);
Error* TemperatureThreshold_ValidateFields(TemperatureThreshold*);

static inline void TemperatureThreshold_Set_UpThreshold(TemperatureThreshold* o) {
	o->_set |= (1 << 0);
}

static inline void TemperatureThreshold_UnSet_UpThreshold(TemperatureThreshold* o) {
	o->_set &= ~(1 << 0);
}

static inline bool TemperatureThreshold_IsSet_UpThreshold(TemperatureThreshold* o) {
	return o->_set & (1 << 0);
}

static inline void TemperatureThreshold_Set_DownThreshold(TemperatureThreshold* o) {
	o->_set |= (1 << 1);
}

static inline void TemperatureThreshold_UnSet_DownThreshold(TemperatureThreshold* o) {
	o->_set &= ~(1 << 1);
}

static inline bool TemperatureThreshold_IsSet_DownThreshold(TemperatureThreshold* o) {
	return o->_set & (1 << 1);
}

static inline void TemperatureThreshold_Set_FanSpeed(TemperatureThreshold* o) {
	o->_set |= (1 << 2);
}

static inline void TemperatureThreshold_UnSet_FanSpeed(TemperatureThreshold* o) {
	o->_set &= ~(1 << 2);
}

static inline bool TemperatureThreshold_IsSet_FanSpeed(TemperatureThreshold* o) {
	return o->_set & (1 << 2);
}

struct FanSpeedPercentageOverride {
	float           FanSpeedPercentage;
	uint16_t        FanSpeedValue;
	OverrideTargetOperation TargetOperation;
	uint8_t         _set;
};

typedef struct FanSpeedPercentageOverride FanSpeedPercentageOverride;
declare_array_of(FanSpeedPercentageOverride);
Error* FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride*, const nx_json*);
Error* FanSpeedPercentageOverride_ValidateFields(FanSpeedPercentageOverride*);

static inline void FanSpeedPercentageOverride_Set_FanSpeedPercentage(FanSpeedPercentageOverride* o) {
	o->_set |= (1 << 0);
}

static inline void FanSpeedPercentageOverride_UnSet_FanSpeedPercentage(FanSpeedPercentageOverride* o) {
	o->_set &= ~(1 << 0);
}

static inline bool FanSpeedPercentageOverride_IsSet_FanSpeedPercentage(FanSpeedPercentageOverride* o) {
	return o->_set & (1 << 0);
}

static inline void FanSpeedPercentageOverride_Set_FanSpeedValue(FanSpeedPercentageOverride* o) {
	o->_set |= (1 << 1);
}

static inline void FanSpeedPercentageOverride_UnSet_FanSpeedValue(FanSpeedPercentageOverride* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanSpeedPercentageOverride_IsSet_FanSpeedValue(FanSpeedPercentageOverride* o) {
	return o->_set & (1 << 1);
}

static inline void FanSpeedPercentageOverride_Set_TargetOperation(FanSpeedPercentageOverride* o) {
	o->_set |= (1 << 2);
}

static inline void FanSpeedPercentageOverride_UnSet_TargetOperation(FanSpeedPercentageOverride* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanSpeedPercentageOverride_IsSet_TargetOperation(FanSpeedPercentageOverride* o) {
	return o->_set & (1 << 2);
}

struct RegisterWriteConfiguration {
	RegisterWriteMode WriteMode;
	RegisterWriteOccasion WriteOccasion;
	uint8_t         Register;
	uint16_t        Value;
	bool            ResetRequired;
	uint16_t        ResetValue;
	RegisterWriteMode ResetWriteMode;
	const char*     Description;
	uint16_t        _set;
};

typedef struct RegisterWriteConfiguration RegisterWriteConfiguration;
declare_array_of(RegisterWriteConfiguration);
Error* RegisterWriteConfiguration_FromJson(RegisterWriteConfiguration*, const nx_json*);
Error* RegisterWriteConfiguration_ValidateFields(RegisterWriteConfiguration*);

static inline void RegisterWriteConfiguration_Set_WriteMode(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 0);
}

static inline void RegisterWriteConfiguration_UnSet_WriteMode(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 0);
}

static inline bool RegisterWriteConfiguration_IsSet_WriteMode(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 0);
}

static inline void RegisterWriteConfiguration_Set_WriteOccasion(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 1);
}

static inline void RegisterWriteConfiguration_UnSet_WriteOccasion(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 1);
}

static inline bool RegisterWriteConfiguration_IsSet_WriteOccasion(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 1);
}

static inline void RegisterWriteConfiguration_Set_Register(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 2);
}

static inline void RegisterWriteConfiguration_UnSet_Register(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 2);
}

static inline bool RegisterWriteConfiguration_IsSet_Register(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 2);
}

static inline void RegisterWriteConfiguration_Set_Value(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 3);
}

static inline void RegisterWriteConfiguration_UnSet_Value(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 3);
}

static inline bool RegisterWriteConfiguration_IsSet_Value(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 3);
}

static inline void RegisterWriteConfiguration_Set_ResetRequired(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 4);
}

static inline void RegisterWriteConfiguration_UnSet_ResetRequired(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 4);
}

static inline bool RegisterWriteConfiguration_IsSet_ResetRequired(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 4);
}

static inline void RegisterWriteConfiguration_Set_ResetValue(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 5);
}

static inline void RegisterWriteConfiguration_UnSet_ResetValue(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 5);
}

static inline bool RegisterWriteConfiguration_IsSet_ResetValue(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 5);
}

static inline void RegisterWriteConfiguration_Set_ResetWriteMode(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 6);
}

static inline void RegisterWriteConfiguration_UnSet_ResetWriteMode(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 6);
}

static inline bool RegisterWriteConfiguration_IsSet_ResetWriteMode(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 6);
}

static inline void RegisterWriteConfiguration_Set_Description(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 7);
}

static inline void RegisterWriteConfiguration_UnSet_Description(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 7);
}

static inline bool RegisterWriteConfiguration_IsSet_Description(RegisterWriteConfiguration* o) {
	return o->_set & (1 << 7);
}

struct FanConfiguration {
	const char*     FanDisplayName;
	uint8_t         ReadRegister;
	uint8_t         WriteRegister;
	uint16_t        MinSpeedValue;
	uint16_t        MaxSpeedValue;
	uint16_t        MinSpeedValueRead;
	uint16_t        MaxSpeedValueRead;
	bool            IndependentReadMinMaxValues;
	bool            ResetRequired;
	uint16_t        FanSpeedResetValue;
	TemperatureAlgorithmType TemperatureAlgorithmType;
	array_of(str)   Sensors;
	array_of(TemperatureThreshold) TemperatureThresholds;
	array_of(FanSpeedPercentageOverride) FanSpeedPercentageOverrides;
	uint16_t        _set;
};

typedef struct FanConfiguration FanConfiguration;
declare_array_of(FanConfiguration);
Error* FanConfiguration_FromJson(FanConfiguration*, const nx_json*);
Error* FanConfiguration_ValidateFields(FanConfiguration*);

static inline void FanConfiguration_Set_FanDisplayName(FanConfiguration* o) {
	o->_set |= (1 << 0);
}

static inline void FanConfiguration_UnSet_FanDisplayName(FanConfiguration* o) {
	o->_set &= ~(1 << 0);
}

static inline bool FanConfiguration_IsSet_FanDisplayName(FanConfiguration* o) {
	return o->_set & (1 << 0);
}

static inline void FanConfiguration_Set_ReadRegister(FanConfiguration* o) {
	o->_set |= (1 << 1);
}

static inline void FanConfiguration_UnSet_ReadRegister(FanConfiguration* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanConfiguration_IsSet_ReadRegister(FanConfiguration* o) {
	return o->_set & (1 << 1);
}

static inline void FanConfiguration_Set_WriteRegister(FanConfiguration* o) {
	o->_set |= (1 << 2);
}

static inline void FanConfiguration_UnSet_WriteRegister(FanConfiguration* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanConfiguration_IsSet_WriteRegister(FanConfiguration* o) {
	return o->_set & (1 << 2);
}

static inline void FanConfiguration_Set_MinSpeedValue(FanConfiguration* o) {
	o->_set |= (1 << 3);
}

static inline void FanConfiguration_UnSet_MinSpeedValue(FanConfiguration* o) {
	o->_set &= ~(1 << 3);
}

static inline bool FanConfiguration_IsSet_MinSpeedValue(FanConfiguration* o) {
	return o->_set & (1 << 3);
}

static inline void FanConfiguration_Set_MaxSpeedValue(FanConfiguration* o) {
	o->_set |= (1 << 4);
}

static inline void FanConfiguration_UnSet_MaxSpeedValue(FanConfiguration* o) {
	o->_set &= ~(1 << 4);
}

static inline bool FanConfiguration_IsSet_MaxSpeedValue(FanConfiguration* o) {
	return o->_set & (1 << 4);
}

static inline void FanConfiguration_Set_MinSpeedValueRead(FanConfiguration* o) {
	o->_set |= (1 << 5);
}

static inline void FanConfiguration_UnSet_MinSpeedValueRead(FanConfiguration* o) {
	o->_set &= ~(1 << 5);
}

static inline bool FanConfiguration_IsSet_MinSpeedValueRead(FanConfiguration* o) {
	return o->_set & (1 << 5);
}

static inline void FanConfiguration_Set_MaxSpeedValueRead(FanConfiguration* o) {
	o->_set |= (1 << 6);
}

static inline void FanConfiguration_UnSet_MaxSpeedValueRead(FanConfiguration* o) {
	o->_set &= ~(1 << 6);
}

static inline bool FanConfiguration_IsSet_MaxSpeedValueRead(FanConfiguration* o) {
	return o->_set & (1 << 6);
}

static inline void FanConfiguration_Set_IndependentReadMinMaxValues(FanConfiguration* o) {
	o->_set |= (1 << 7);
}

static inline void FanConfiguration_UnSet_IndependentReadMinMaxValues(FanConfiguration* o) {
	o->_set &= ~(1 << 7);
}

static inline bool FanConfiguration_IsSet_IndependentReadMinMaxValues(FanConfiguration* o) {
	return o->_set & (1 << 7);
}

static inline void FanConfiguration_Set_ResetRequired(FanConfiguration* o) {
	o->_set |= (1 << 8);
}

static inline void FanConfiguration_UnSet_ResetRequired(FanConfiguration* o) {
	o->_set &= ~(1 << 8);
}

static inline bool FanConfiguration_IsSet_ResetRequired(FanConfiguration* o) {
	return o->_set & (1 << 8);
}

static inline void FanConfiguration_Set_FanSpeedResetValue(FanConfiguration* o) {
	o->_set |= (1 << 9);
}

static inline void FanConfiguration_UnSet_FanSpeedResetValue(FanConfiguration* o) {
	o->_set &= ~(1 << 9);
}

static inline bool FanConfiguration_IsSet_FanSpeedResetValue(FanConfiguration* o) {
	return o->_set & (1 << 9);
}

static inline void FanConfiguration_Set_TemperatureAlgorithmType(FanConfiguration* o) {
	o->_set |= (1 << 10);
}

static inline void FanConfiguration_UnSet_TemperatureAlgorithmType(FanConfiguration* o) {
	o->_set &= ~(1 << 10);
}

static inline bool FanConfiguration_IsSet_TemperatureAlgorithmType(FanConfiguration* o) {
	return o->_set & (1 << 10);
}

static inline void FanConfiguration_Set_Sensors(FanConfiguration* o) {
	o->_set |= (1 << 11);
}

static inline void FanConfiguration_UnSet_Sensors(FanConfiguration* o) {
	o->_set &= ~(1 << 11);
}

static inline bool FanConfiguration_IsSet_Sensors(FanConfiguration* o) {
	return o->_set & (1 << 11);
}

static inline void FanConfiguration_Set_TemperatureThresholds(FanConfiguration* o) {
	o->_set |= (1 << 12);
}

static inline void FanConfiguration_UnSet_TemperatureThresholds(FanConfiguration* o) {
	o->_set &= ~(1 << 12);
}

static inline bool FanConfiguration_IsSet_TemperatureThresholds(FanConfiguration* o) {
	return o->_set & (1 << 12);
}

static inline void FanConfiguration_Set_FanSpeedPercentageOverrides(FanConfiguration* o) {
	o->_set |= (1 << 13);
}

static inline void FanConfiguration_UnSet_FanSpeedPercentageOverrides(FanConfiguration* o) {
	o->_set &= ~(1 << 13);
}

static inline bool FanConfiguration_IsSet_FanSpeedPercentageOverrides(FanConfiguration* o) {
	return o->_set & (1 << 13);
}

struct ModelConfig {
	const char*     NotebookModel;
	const char*     Author;
	bool            LegacyTemperatureThresholdsBehaviour;
	uint16_t        EcPollInterval;
	int16_t         CriticalTemperature;
	uint16_t        CriticalTemperatureOffset;
	bool            ReadWriteWords;
	array_of(FanConfiguration) FanConfigurations;
	array_of(RegisterWriteConfiguration) RegisterWriteConfigurations;
	uint16_t        _set;
};

typedef struct ModelConfig ModelConfig;
declare_array_of(ModelConfig);
Error* ModelConfig_FromJson(ModelConfig*, const nx_json*);
Error* ModelConfig_ValidateFields(ModelConfig*);

static inline void ModelConfig_Set_NotebookModel(ModelConfig* o) {
	o->_set |= (1 << 0);
}

static inline void ModelConfig_UnSet_NotebookModel(ModelConfig* o) {
	o->_set &= ~(1 << 0);
}

static inline bool ModelConfig_IsSet_NotebookModel(ModelConfig* o) {
	return o->_set & (1 << 0);
}

static inline void ModelConfig_Set_Author(ModelConfig* o) {
	o->_set |= (1 << 1);
}

static inline void ModelConfig_UnSet_Author(ModelConfig* o) {
	o->_set &= ~(1 << 1);
}

static inline bool ModelConfig_IsSet_Author(ModelConfig* o) {
	return o->_set & (1 << 1);
}

static inline void ModelConfig_Set_LegacyTemperatureThresholdsBehaviour(ModelConfig* o) {
	o->_set |= (1 << 2);
}

static inline void ModelConfig_UnSet_LegacyTemperatureThresholdsBehaviour(ModelConfig* o) {
	o->_set &= ~(1 << 2);
}

static inline bool ModelConfig_IsSet_LegacyTemperatureThresholdsBehaviour(ModelConfig* o) {
	return o->_set & (1 << 2);
}

static inline void ModelConfig_Set_EcPollInterval(ModelConfig* o) {
	o->_set |= (1 << 3);
}

static inline void ModelConfig_UnSet_EcPollInterval(ModelConfig* o) {
	o->_set &= ~(1 << 3);
}

static inline bool ModelConfig_IsSet_EcPollInterval(ModelConfig* o) {
	return o->_set & (1 << 3);
}

static inline void ModelConfig_Set_CriticalTemperature(ModelConfig* o) {
	o->_set |= (1 << 4);
}

static inline void ModelConfig_UnSet_CriticalTemperature(ModelConfig* o) {
	o->_set &= ~(1 << 4);
}

static inline bool ModelConfig_IsSet_CriticalTemperature(ModelConfig* o) {
	return o->_set & (1 << 4);
}

static inline void ModelConfig_Set_CriticalTemperatureOffset(ModelConfig* o) {
	o->_set |= (1 << 5);
}

static inline void ModelConfig_UnSet_CriticalTemperatureOffset(ModelConfig* o) {
	o->_set &= ~(1 << 5);
}

static inline bool ModelConfig_IsSet_CriticalTemperatureOffset(ModelConfig* o) {
	return o->_set & (1 << 5);
}

static inline void ModelConfig_Set_ReadWriteWords(ModelConfig* o) {
	o->_set |= (1 << 6);
}

static inline void ModelConfig_UnSet_ReadWriteWords(ModelConfig* o) {
	o->_set &= ~(1 << 6);
}

static inline bool ModelConfig_IsSet_ReadWriteWords(ModelConfig* o) {
	return o->_set & (1 << 6);
}

static inline void ModelConfig_Set_FanConfigurations(ModelConfig* o) {
	o->_set |= (1 << 7);
}

static inline void ModelConfig_UnSet_FanConfigurations(ModelConfig* o) {
	o->_set &= ~(1 << 7);
}

static inline bool ModelConfig_IsSet_FanConfigurations(ModelConfig* o) {
	return o->_set & (1 << 7);
}

static inline void ModelConfig_Set_RegisterWriteConfigurations(ModelConfig* o) {
	o->_set |= (1 << 8);
}

static inline void ModelConfig_UnSet_RegisterWriteConfigurations(ModelConfig* o) {
	o->_set &= ~(1 << 8);
}

static inline bool ModelConfig_IsSet_RegisterWriteConfigurations(ModelConfig* o) {
	return o->_set & (1 << 8);
}

struct FanTemperatureSourceConfig {
	uint8_t         FanIndex;
	TemperatureAlgorithmType TemperatureAlgorithmType;
	array_of(str)   Sensors;
	uint8_t         _set;
};

typedef struct FanTemperatureSourceConfig FanTemperatureSourceConfig;
declare_array_of(FanTemperatureSourceConfig);
Error* FanTemperatureSourceConfig_FromJson(FanTemperatureSourceConfig*, const nx_json*);
Error* FanTemperatureSourceConfig_ValidateFields(FanTemperatureSourceConfig*);

static inline void FanTemperatureSourceConfig_Set_FanIndex(FanTemperatureSourceConfig* o) {
	o->_set |= (1 << 0);
}

static inline void FanTemperatureSourceConfig_UnSet_FanIndex(FanTemperatureSourceConfig* o) {
	o->_set &= ~(1 << 0);
}

static inline bool FanTemperatureSourceConfig_IsSet_FanIndex(FanTemperatureSourceConfig* o) {
	return o->_set & (1 << 0);
}

static inline void FanTemperatureSourceConfig_Set_TemperatureAlgorithmType(FanTemperatureSourceConfig* o) {
	o->_set |= (1 << 1);
}

static inline void FanTemperatureSourceConfig_UnSet_TemperatureAlgorithmType(FanTemperatureSourceConfig* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanTemperatureSourceConfig_IsSet_TemperatureAlgorithmType(FanTemperatureSourceConfig* o) {
	return o->_set & (1 << 1);
}

static inline void FanTemperatureSourceConfig_Set_Sensors(FanTemperatureSourceConfig* o) {
	o->_set |= (1 << 2);
}

static inline void FanTemperatureSourceConfig_UnSet_Sensors(FanTemperatureSourceConfig* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanTemperatureSourceConfig_IsSet_Sensors(FanTemperatureSourceConfig* o) {
	return o->_set & (1 << 2);
}

struct ServiceConfig {
	const char*     SelectedConfigId;
	EmbeddedControllerType EmbeddedControllerType;
	array_of(float) TargetFanSpeeds;
	array_of(FanTemperatureSourceConfig) FanTemperatureSources;
	uint8_t         _set;
};

typedef struct ServiceConfig ServiceConfig;
declare_array_of(ServiceConfig);
Error* ServiceConfig_FromJson(ServiceConfig*, const nx_json*);
Error* ServiceConfig_ValidateFields(ServiceConfig*);

static inline void ServiceConfig_Set_SelectedConfigId(ServiceConfig* o) {
	o->_set |= (1 << 0);
}

static inline void ServiceConfig_UnSet_SelectedConfigId(ServiceConfig* o) {
	o->_set &= ~(1 << 0);
}

static inline bool ServiceConfig_IsSet_SelectedConfigId(ServiceConfig* o) {
	return o->_set & (1 << 0);
}

static inline void ServiceConfig_Set_EmbeddedControllerType(ServiceConfig* o) {
	o->_set |= (1 << 1);
}

static inline void ServiceConfig_UnSet_EmbeddedControllerType(ServiceConfig* o) {
	o->_set &= ~(1 << 1);
}

static inline bool ServiceConfig_IsSet_EmbeddedControllerType(ServiceConfig* o) {
	return o->_set & (1 << 1);
}

static inline void ServiceConfig_Set_TargetFanSpeeds(ServiceConfig* o) {
	o->_set |= (1 << 2);
}

static inline void ServiceConfig_UnSet_TargetFanSpeeds(ServiceConfig* o) {
	o->_set &= ~(1 << 2);
}

static inline bool ServiceConfig_IsSet_TargetFanSpeeds(ServiceConfig* o) {
	return o->_set & (1 << 2);
}

static inline void ServiceConfig_Set_FanTemperatureSources(ServiceConfig* o) {
	o->_set |= (1 << 3);
}

static inline void ServiceConfig_UnSet_FanTemperatureSources(ServiceConfig* o) {
	o->_set &= ~(1 << 3);
}

static inline bool ServiceConfig_IsSet_FanTemperatureSources(ServiceConfig* o) {
	return o->_set & (1 << 3);
}

struct FanInfo {
	const char*     Name;
	float           Temperature;
	bool            AutoMode;
	bool            Critical;
	float           CurrentSpeed;
	float           TargetSpeed;
	float           RequestedSpeed;
	uint16_t        SpeedSteps;
	uint16_t        _set;
};

typedef struct FanInfo FanInfo;
declare_array_of(FanInfo);
Error* FanInfo_FromJson(FanInfo*, const nx_json*);
Error* FanInfo_ValidateFields(FanInfo*);

static inline void FanInfo_Set_Name(FanInfo* o) {
	o->_set |= (1 << 0);
}

static inline void FanInfo_UnSet_Name(FanInfo* o) {
	o->_set &= ~(1 << 0);
}

static inline bool FanInfo_IsSet_Name(FanInfo* o) {
	return o->_set & (1 << 0);
}

static inline void FanInfo_Set_Temperature(FanInfo* o) {
	o->_set |= (1 << 1);
}

static inline void FanInfo_UnSet_Temperature(FanInfo* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanInfo_IsSet_Temperature(FanInfo* o) {
	return o->_set & (1 << 1);
}

static inline void FanInfo_Set_AutoMode(FanInfo* o) {
	o->_set |= (1 << 2);
}

static inline void FanInfo_UnSet_AutoMode(FanInfo* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanInfo_IsSet_AutoMode(FanInfo* o) {
	return o->_set & (1 << 2);
}

static inline void FanInfo_Set_Critical(FanInfo* o) {
	o->_set |= (1 << 3);
}

static inline void FanInfo_UnSet_Critical(FanInfo* o) {
	o->_set &= ~(1 << 3);
}

static inline bool FanInfo_IsSet_Critical(FanInfo* o) {
	return o->_set & (1 << 3);
}

static inline void FanInfo_Set_CurrentSpeed(FanInfo* o) {
	o->_set |= (1 << 4);
}

static inline void FanInfo_UnSet_CurrentSpeed(FanInfo* o) {
	o->_set &= ~(1 << 4);
}

static inline bool FanInfo_IsSet_CurrentSpeed(FanInfo* o) {
	return o->_set & (1 << 4);
}

static inline void FanInfo_Set_TargetSpeed(FanInfo* o) {
	o->_set |= (1 << 5);
}

static inline void FanInfo_UnSet_TargetSpeed(FanInfo* o) {
	o->_set &= ~(1 << 5);
}

static inline bool FanInfo_IsSet_TargetSpeed(FanInfo* o) {
	return o->_set & (1 << 5);
}

static inline void FanInfo_Set_RequestedSpeed(FanInfo* o) {
	o->_set |= (1 << 6);
}

static inline void FanInfo_UnSet_RequestedSpeed(FanInfo* o) {
	o->_set &= ~(1 << 6);
}

static inline bool FanInfo_IsSet_RequestedSpeed(FanInfo* o) {
	return o->_set & (1 << 6);
}

static inline void FanInfo_Set_SpeedSteps(FanInfo* o) {
	o->_set |= (1 << 7);
}

static inline void FanInfo_UnSet_SpeedSteps(FanInfo* o) {
	o->_set &= ~(1 << 7);
}

static inline bool FanInfo_IsSet_SpeedSteps(FanInfo* o) {
	return o->_set & (1 << 7);
}

struct ServiceInfo {
	int             PID;
	const char*     SelectedConfigId;
	bool            ReadOnly;
	array_of(FanInfo) Fans;
	uint8_t         _set;
};

typedef struct ServiceInfo ServiceInfo;
declare_array_of(ServiceInfo);
Error* ServiceInfo_FromJson(ServiceInfo*, const nx_json*);
Error* ServiceInfo_ValidateFields(ServiceInfo*);

static inline void ServiceInfo_Set_PID(ServiceInfo* o) {
	o->_set |= (1 << 0);
}

static inline void ServiceInfo_UnSet_PID(ServiceInfo* o) {
	o->_set &= ~(1 << 0);
}

static inline bool ServiceInfo_IsSet_PID(ServiceInfo* o) {
	return o->_set & (1 << 0);
}

static inline void ServiceInfo_Set_SelectedConfigId(ServiceInfo* o) {
	o->_set |= (1 << 1);
}

static inline void ServiceInfo_UnSet_SelectedConfigId(ServiceInfo* o) {
	o->_set &= ~(1 << 1);
}

static inline bool ServiceInfo_IsSet_SelectedConfigId(ServiceInfo* o) {
	return o->_set & (1 << 1);
}

static inline void ServiceInfo_Set_ReadOnly(ServiceInfo* o) {
	o->_set |= (1 << 2);
}

static inline void ServiceInfo_UnSet_ReadOnly(ServiceInfo* o) {
	o->_set &= ~(1 << 2);
}

static inline bool ServiceInfo_IsSet_ReadOnly(ServiceInfo* o) {
	return o->_set & (1 << 2);
}

static inline void ServiceInfo_Set_Fans(ServiceInfo* o) {
	o->_set |= (1 << 3);
}

static inline void ServiceInfo_UnSet_Fans(ServiceInfo* o) {
	o->_set &= ~(1 << 3);
}

static inline bool ServiceInfo_IsSet_Fans(ServiceInfo* o) {
	return o->_set & (1 << 3);
}

