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

static inline bool TemperatureThreshold_IsSet_UpThreshold(const TemperatureThreshold* o) {
	return o->_set & (1 << 0);
}

static inline void TemperatureThreshold_Set_DownThreshold(TemperatureThreshold* o) {
	o->_set |= (1 << 1);
}

static inline void TemperatureThreshold_UnSet_DownThreshold(TemperatureThreshold* o) {
	o->_set &= ~(1 << 1);
}

static inline bool TemperatureThreshold_IsSet_DownThreshold(const TemperatureThreshold* o) {
	return o->_set & (1 << 1);
}

static inline void TemperatureThreshold_Set_FanSpeed(TemperatureThreshold* o) {
	o->_set |= (1 << 2);
}

static inline void TemperatureThreshold_UnSet_FanSpeed(TemperatureThreshold* o) {
	o->_set &= ~(1 << 2);
}

static inline bool TemperatureThreshold_IsSet_FanSpeed(const TemperatureThreshold* o) {
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

static inline bool FanSpeedPercentageOverride_IsSet_FanSpeedPercentage(const FanSpeedPercentageOverride* o) {
	return o->_set & (1 << 0);
}

static inline void FanSpeedPercentageOverride_Set_FanSpeedValue(FanSpeedPercentageOverride* o) {
	o->_set |= (1 << 1);
}

static inline void FanSpeedPercentageOverride_UnSet_FanSpeedValue(FanSpeedPercentageOverride* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanSpeedPercentageOverride_IsSet_FanSpeedValue(const FanSpeedPercentageOverride* o) {
	return o->_set & (1 << 1);
}

static inline void FanSpeedPercentageOverride_Set_TargetOperation(FanSpeedPercentageOverride* o) {
	o->_set |= (1 << 2);
}

static inline void FanSpeedPercentageOverride_UnSet_TargetOperation(FanSpeedPercentageOverride* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanSpeedPercentageOverride_IsSet_TargetOperation(const FanSpeedPercentageOverride* o) {
	return o->_set & (1 << 2);
}

struct RegisterWriteConfiguration {
	RegisterWriteMode WriteMode;
	RegisterWriteOccasion WriteOccasion;
	uint8_t         Register;
	uint16_t        Value;
	const char*     AcpiMethod;
	bool            ResetRequired;
	uint16_t        ResetValue;
	const char*     ResetAcpiMethod;
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

static inline bool RegisterWriteConfiguration_IsSet_WriteMode(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 0);
}

static inline void RegisterWriteConfiguration_Set_WriteOccasion(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 1);
}

static inline void RegisterWriteConfiguration_UnSet_WriteOccasion(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 1);
}

static inline bool RegisterWriteConfiguration_IsSet_WriteOccasion(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 1);
}

static inline void RegisterWriteConfiguration_Set_Register(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 2);
}

static inline void RegisterWriteConfiguration_UnSet_Register(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 2);
}

static inline bool RegisterWriteConfiguration_IsSet_Register(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 2);
}

static inline void RegisterWriteConfiguration_Set_Value(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 3);
}

static inline void RegisterWriteConfiguration_UnSet_Value(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 3);
}

static inline bool RegisterWriteConfiguration_IsSet_Value(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 3);
}

static inline void RegisterWriteConfiguration_Set_AcpiMethod(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 4);
}

static inline void RegisterWriteConfiguration_UnSet_AcpiMethod(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 4);
}

static inline bool RegisterWriteConfiguration_IsSet_AcpiMethod(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 4);
}

static inline void RegisterWriteConfiguration_Set_ResetRequired(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 5);
}

static inline void RegisterWriteConfiguration_UnSet_ResetRequired(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 5);
}

static inline bool RegisterWriteConfiguration_IsSet_ResetRequired(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 5);
}

static inline void RegisterWriteConfiguration_Set_ResetValue(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 6);
}

static inline void RegisterWriteConfiguration_UnSet_ResetValue(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 6);
}

static inline bool RegisterWriteConfiguration_IsSet_ResetValue(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 6);
}

static inline void RegisterWriteConfiguration_Set_ResetAcpiMethod(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 7);
}

static inline void RegisterWriteConfiguration_UnSet_ResetAcpiMethod(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 7);
}

static inline bool RegisterWriteConfiguration_IsSet_ResetAcpiMethod(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 7);
}

static inline void RegisterWriteConfiguration_Set_ResetWriteMode(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 8);
}

static inline void RegisterWriteConfiguration_UnSet_ResetWriteMode(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 8);
}

static inline bool RegisterWriteConfiguration_IsSet_ResetWriteMode(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 8);
}

static inline void RegisterWriteConfiguration_Set_Description(RegisterWriteConfiguration* o) {
	o->_set |= (1 << 9);
}

static inline void RegisterWriteConfiguration_UnSet_Description(RegisterWriteConfiguration* o) {
	o->_set &= ~(1 << 9);
}

static inline bool RegisterWriteConfiguration_IsSet_Description(const RegisterWriteConfiguration* o) {
	return o->_set & (1 << 9);
}

struct FanConfiguration {
	const char*     FanDisplayName;
	uint8_t         ReadRegister;
	const char*     ReadAcpiMethod;
	uint8_t         WriteRegister;
	const char*     WriteAcpiMethod;
	uint16_t        MinSpeedValue;
	uint16_t        MaxSpeedValue;
	uint16_t        MinSpeedValueRead;
	uint16_t        MaxSpeedValueRead;
	bool            IndependentReadMinMaxValues;
	bool            ResetRequired;
	uint16_t        FanSpeedResetValue;
	const char*     ResetAcpiMethod;
	TemperatureAlgorithmType TemperatureAlgorithmType;
	array_of(str)   Sensors;
	array_of(TemperatureThreshold) TemperatureThresholds;
	array_of(FanSpeedPercentageOverride) FanSpeedPercentageOverrides;
	uint32_t        _set;
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

static inline bool FanConfiguration_IsSet_FanDisplayName(const FanConfiguration* o) {
	return o->_set & (1 << 0);
}

static inline void FanConfiguration_Set_ReadRegister(FanConfiguration* o) {
	o->_set |= (1 << 1);
}

static inline void FanConfiguration_UnSet_ReadRegister(FanConfiguration* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanConfiguration_IsSet_ReadRegister(const FanConfiguration* o) {
	return o->_set & (1 << 1);
}

static inline void FanConfiguration_Set_ReadAcpiMethod(FanConfiguration* o) {
	o->_set |= (1 << 2);
}

static inline void FanConfiguration_UnSet_ReadAcpiMethod(FanConfiguration* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanConfiguration_IsSet_ReadAcpiMethod(const FanConfiguration* o) {
	return o->_set & (1 << 2);
}

static inline void FanConfiguration_Set_WriteRegister(FanConfiguration* o) {
	o->_set |= (1 << 3);
}

static inline void FanConfiguration_UnSet_WriteRegister(FanConfiguration* o) {
	o->_set &= ~(1 << 3);
}

static inline bool FanConfiguration_IsSet_WriteRegister(const FanConfiguration* o) {
	return o->_set & (1 << 3);
}

static inline void FanConfiguration_Set_WriteAcpiMethod(FanConfiguration* o) {
	o->_set |= (1 << 4);
}

static inline void FanConfiguration_UnSet_WriteAcpiMethod(FanConfiguration* o) {
	o->_set &= ~(1 << 4);
}

static inline bool FanConfiguration_IsSet_WriteAcpiMethod(const FanConfiguration* o) {
	return o->_set & (1 << 4);
}

static inline void FanConfiguration_Set_MinSpeedValue(FanConfiguration* o) {
	o->_set |= (1 << 5);
}

static inline void FanConfiguration_UnSet_MinSpeedValue(FanConfiguration* o) {
	o->_set &= ~(1 << 5);
}

static inline bool FanConfiguration_IsSet_MinSpeedValue(const FanConfiguration* o) {
	return o->_set & (1 << 5);
}

static inline void FanConfiguration_Set_MaxSpeedValue(FanConfiguration* o) {
	o->_set |= (1 << 6);
}

static inline void FanConfiguration_UnSet_MaxSpeedValue(FanConfiguration* o) {
	o->_set &= ~(1 << 6);
}

static inline bool FanConfiguration_IsSet_MaxSpeedValue(const FanConfiguration* o) {
	return o->_set & (1 << 6);
}

static inline void FanConfiguration_Set_MinSpeedValueRead(FanConfiguration* o) {
	o->_set |= (1 << 7);
}

static inline void FanConfiguration_UnSet_MinSpeedValueRead(FanConfiguration* o) {
	o->_set &= ~(1 << 7);
}

static inline bool FanConfiguration_IsSet_MinSpeedValueRead(const FanConfiguration* o) {
	return o->_set & (1 << 7);
}

static inline void FanConfiguration_Set_MaxSpeedValueRead(FanConfiguration* o) {
	o->_set |= (1 << 8);
}

static inline void FanConfiguration_UnSet_MaxSpeedValueRead(FanConfiguration* o) {
	o->_set &= ~(1 << 8);
}

static inline bool FanConfiguration_IsSet_MaxSpeedValueRead(const FanConfiguration* o) {
	return o->_set & (1 << 8);
}

static inline void FanConfiguration_Set_IndependentReadMinMaxValues(FanConfiguration* o) {
	o->_set |= (1 << 9);
}

static inline void FanConfiguration_UnSet_IndependentReadMinMaxValues(FanConfiguration* o) {
	o->_set &= ~(1 << 9);
}

static inline bool FanConfiguration_IsSet_IndependentReadMinMaxValues(const FanConfiguration* o) {
	return o->_set & (1 << 9);
}

static inline void FanConfiguration_Set_ResetRequired(FanConfiguration* o) {
	o->_set |= (1 << 10);
}

static inline void FanConfiguration_UnSet_ResetRequired(FanConfiguration* o) {
	o->_set &= ~(1 << 10);
}

static inline bool FanConfiguration_IsSet_ResetRequired(const FanConfiguration* o) {
	return o->_set & (1 << 10);
}

static inline void FanConfiguration_Set_FanSpeedResetValue(FanConfiguration* o) {
	o->_set |= (1 << 11);
}

static inline void FanConfiguration_UnSet_FanSpeedResetValue(FanConfiguration* o) {
	o->_set &= ~(1 << 11);
}

static inline bool FanConfiguration_IsSet_FanSpeedResetValue(const FanConfiguration* o) {
	return o->_set & (1 << 11);
}

static inline void FanConfiguration_Set_ResetAcpiMethod(FanConfiguration* o) {
	o->_set |= (1 << 12);
}

static inline void FanConfiguration_UnSet_ResetAcpiMethod(FanConfiguration* o) {
	o->_set &= ~(1 << 12);
}

static inline bool FanConfiguration_IsSet_ResetAcpiMethod(const FanConfiguration* o) {
	return o->_set & (1 << 12);
}

static inline void FanConfiguration_Set_TemperatureAlgorithmType(FanConfiguration* o) {
	o->_set |= (1 << 13);
}

static inline void FanConfiguration_UnSet_TemperatureAlgorithmType(FanConfiguration* o) {
	o->_set &= ~(1 << 13);
}

static inline bool FanConfiguration_IsSet_TemperatureAlgorithmType(const FanConfiguration* o) {
	return o->_set & (1 << 13);
}

static inline void FanConfiguration_Set_Sensors(FanConfiguration* o) {
	o->_set |= (1 << 14);
}

static inline void FanConfiguration_UnSet_Sensors(FanConfiguration* o) {
	o->_set &= ~(1 << 14);
}

static inline bool FanConfiguration_IsSet_Sensors(const FanConfiguration* o) {
	return o->_set & (1 << 14);
}

static inline void FanConfiguration_Set_TemperatureThresholds(FanConfiguration* o) {
	o->_set |= (1 << 15);
}

static inline void FanConfiguration_UnSet_TemperatureThresholds(FanConfiguration* o) {
	o->_set &= ~(1 << 15);
}

static inline bool FanConfiguration_IsSet_TemperatureThresholds(const FanConfiguration* o) {
	return o->_set & (1 << 15);
}

static inline void FanConfiguration_Set_FanSpeedPercentageOverrides(FanConfiguration* o) {
	o->_set |= (1 << 16);
}

static inline void FanConfiguration_UnSet_FanSpeedPercentageOverrides(FanConfiguration* o) {
	o->_set &= ~(1 << 16);
}

static inline bool FanConfiguration_IsSet_FanSpeedPercentageOverrides(const FanConfiguration* o) {
	return o->_set & (1 << 16);
}

struct Sponsor {
	const char*     Name;
	const char*     Description;
	const char*     URL;
	const char*     BannerURL;
	uint8_t         _set;
};

typedef struct Sponsor Sponsor;
declare_array_of(Sponsor);
Error* Sponsor_FromJson(Sponsor*, const nx_json*);
Error* Sponsor_ValidateFields(Sponsor*);

static inline void Sponsor_Set_Name(Sponsor* o) {
	o->_set |= (1 << 0);
}

static inline void Sponsor_UnSet_Name(Sponsor* o) {
	o->_set &= ~(1 << 0);
}

static inline bool Sponsor_IsSet_Name(const Sponsor* o) {
	return o->_set & (1 << 0);
}

static inline void Sponsor_Set_Description(Sponsor* o) {
	o->_set |= (1 << 1);
}

static inline void Sponsor_UnSet_Description(Sponsor* o) {
	o->_set &= ~(1 << 1);
}

static inline bool Sponsor_IsSet_Description(const Sponsor* o) {
	return o->_set & (1 << 1);
}

static inline void Sponsor_Set_URL(Sponsor* o) {
	o->_set |= (1 << 2);
}

static inline void Sponsor_UnSet_URL(Sponsor* o) {
	o->_set &= ~(1 << 2);
}

static inline bool Sponsor_IsSet_URL(const Sponsor* o) {
	return o->_set & (1 << 2);
}

static inline void Sponsor_Set_BannerURL(Sponsor* o) {
	o->_set |= (1 << 3);
}

static inline void Sponsor_UnSet_BannerURL(Sponsor* o) {
	o->_set &= ~(1 << 3);
}

static inline bool Sponsor_IsSet_BannerURL(const Sponsor* o) {
	return o->_set & (1 << 3);
}

struct ModelConfig {
	const char*     NotebookModel;
	const char*     Author;
	bool            LegacyTemperatureThresholdsBehaviour;
	uint16_t        EcPollInterval;
	int16_t         CriticalTemperature;
	uint16_t        CriticalTemperatureOffset;
	bool            ReadWriteWords;
	Sponsor         Sponsor;
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

static inline bool ModelConfig_IsSet_NotebookModel(const ModelConfig* o) {
	return o->_set & (1 << 0);
}

static inline void ModelConfig_Set_Author(ModelConfig* o) {
	o->_set |= (1 << 1);
}

static inline void ModelConfig_UnSet_Author(ModelConfig* o) {
	o->_set &= ~(1 << 1);
}

static inline bool ModelConfig_IsSet_Author(const ModelConfig* o) {
	return o->_set & (1 << 1);
}

static inline void ModelConfig_Set_LegacyTemperatureThresholdsBehaviour(ModelConfig* o) {
	o->_set |= (1 << 2);
}

static inline void ModelConfig_UnSet_LegacyTemperatureThresholdsBehaviour(ModelConfig* o) {
	o->_set &= ~(1 << 2);
}

static inline bool ModelConfig_IsSet_LegacyTemperatureThresholdsBehaviour(const ModelConfig* o) {
	return o->_set & (1 << 2);
}

static inline void ModelConfig_Set_EcPollInterval(ModelConfig* o) {
	o->_set |= (1 << 3);
}

static inline void ModelConfig_UnSet_EcPollInterval(ModelConfig* o) {
	o->_set &= ~(1 << 3);
}

static inline bool ModelConfig_IsSet_EcPollInterval(const ModelConfig* o) {
	return o->_set & (1 << 3);
}

static inline void ModelConfig_Set_CriticalTemperature(ModelConfig* o) {
	o->_set |= (1 << 4);
}

static inline void ModelConfig_UnSet_CriticalTemperature(ModelConfig* o) {
	o->_set &= ~(1 << 4);
}

static inline bool ModelConfig_IsSet_CriticalTemperature(const ModelConfig* o) {
	return o->_set & (1 << 4);
}

static inline void ModelConfig_Set_CriticalTemperatureOffset(ModelConfig* o) {
	o->_set |= (1 << 5);
}

static inline void ModelConfig_UnSet_CriticalTemperatureOffset(ModelConfig* o) {
	o->_set &= ~(1 << 5);
}

static inline bool ModelConfig_IsSet_CriticalTemperatureOffset(const ModelConfig* o) {
	return o->_set & (1 << 5);
}

static inline void ModelConfig_Set_ReadWriteWords(ModelConfig* o) {
	o->_set |= (1 << 6);
}

static inline void ModelConfig_UnSet_ReadWriteWords(ModelConfig* o) {
	o->_set &= ~(1 << 6);
}

static inline bool ModelConfig_IsSet_ReadWriteWords(const ModelConfig* o) {
	return o->_set & (1 << 6);
}

static inline void ModelConfig_Set_Sponsor(ModelConfig* o) {
	o->_set |= (1 << 7);
}

static inline void ModelConfig_UnSet_Sponsor(ModelConfig* o) {
	o->_set &= ~(1 << 7);
}

static inline bool ModelConfig_IsSet_Sponsor(const ModelConfig* o) {
	return o->_set & (1 << 7);
}

static inline void ModelConfig_Set_FanConfigurations(ModelConfig* o) {
	o->_set |= (1 << 8);
}

static inline void ModelConfig_UnSet_FanConfigurations(ModelConfig* o) {
	o->_set &= ~(1 << 8);
}

static inline bool ModelConfig_IsSet_FanConfigurations(const ModelConfig* o) {
	return o->_set & (1 << 8);
}

static inline void ModelConfig_Set_RegisterWriteConfigurations(ModelConfig* o) {
	o->_set |= (1 << 9);
}

static inline void ModelConfig_UnSet_RegisterWriteConfigurations(ModelConfig* o) {
	o->_set &= ~(1 << 9);
}

static inline bool ModelConfig_IsSet_RegisterWriteConfigurations(const ModelConfig* o) {
	return o->_set & (1 << 9);
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

static inline bool FanTemperatureSourceConfig_IsSet_FanIndex(const FanTemperatureSourceConfig* o) {
	return o->_set & (1 << 0);
}

static inline void FanTemperatureSourceConfig_Set_TemperatureAlgorithmType(FanTemperatureSourceConfig* o) {
	o->_set |= (1 << 1);
}

static inline void FanTemperatureSourceConfig_UnSet_TemperatureAlgorithmType(FanTemperatureSourceConfig* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanTemperatureSourceConfig_IsSet_TemperatureAlgorithmType(const FanTemperatureSourceConfig* o) {
	return o->_set & (1 << 1);
}

static inline void FanTemperatureSourceConfig_Set_Sensors(FanTemperatureSourceConfig* o) {
	o->_set |= (1 << 2);
}

static inline void FanTemperatureSourceConfig_UnSet_Sensors(FanTemperatureSourceConfig* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanTemperatureSourceConfig_IsSet_Sensors(const FanTemperatureSourceConfig* o) {
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

static inline bool ServiceConfig_IsSet_SelectedConfigId(const ServiceConfig* o) {
	return o->_set & (1 << 0);
}

static inline void ServiceConfig_Set_EmbeddedControllerType(ServiceConfig* o) {
	o->_set |= (1 << 1);
}

static inline void ServiceConfig_UnSet_EmbeddedControllerType(ServiceConfig* o) {
	o->_set &= ~(1 << 1);
}

static inline bool ServiceConfig_IsSet_EmbeddedControllerType(const ServiceConfig* o) {
	return o->_set & (1 << 1);
}

static inline void ServiceConfig_Set_TargetFanSpeeds(ServiceConfig* o) {
	o->_set |= (1 << 2);
}

static inline void ServiceConfig_UnSet_TargetFanSpeeds(ServiceConfig* o) {
	o->_set &= ~(1 << 2);
}

static inline bool ServiceConfig_IsSet_TargetFanSpeeds(const ServiceConfig* o) {
	return o->_set & (1 << 2);
}

static inline void ServiceConfig_Set_FanTemperatureSources(ServiceConfig* o) {
	o->_set |= (1 << 3);
}

static inline void ServiceConfig_UnSet_FanTemperatureSources(ServiceConfig* o) {
	o->_set &= ~(1 << 3);
}

static inline bool ServiceConfig_IsSet_FanTemperatureSources(const ServiceConfig* o) {
	return o->_set & (1 << 3);
}

struct ServiceState {
	array_of(float) TargetFanSpeeds;
	uint8_t         _set;
};

typedef struct ServiceState ServiceState;
declare_array_of(ServiceState);
Error* ServiceState_FromJson(ServiceState*, const nx_json*);
Error* ServiceState_ValidateFields(ServiceState*);

static inline void ServiceState_Set_TargetFanSpeeds(ServiceState* o) {
	o->_set |= (1 << 0);
}

static inline void ServiceState_UnSet_TargetFanSpeeds(ServiceState* o) {
	o->_set &= ~(1 << 0);
}

static inline bool ServiceState_IsSet_TargetFanSpeeds(const ServiceState* o) {
	return o->_set & (1 << 0);
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

static inline bool FanInfo_IsSet_Name(const FanInfo* o) {
	return o->_set & (1 << 0);
}

static inline void FanInfo_Set_Temperature(FanInfo* o) {
	o->_set |= (1 << 1);
}

static inline void FanInfo_UnSet_Temperature(FanInfo* o) {
	o->_set &= ~(1 << 1);
}

static inline bool FanInfo_IsSet_Temperature(const FanInfo* o) {
	return o->_set & (1 << 1);
}

static inline void FanInfo_Set_AutoMode(FanInfo* o) {
	o->_set |= (1 << 2);
}

static inline void FanInfo_UnSet_AutoMode(FanInfo* o) {
	o->_set &= ~(1 << 2);
}

static inline bool FanInfo_IsSet_AutoMode(const FanInfo* o) {
	return o->_set & (1 << 2);
}

static inline void FanInfo_Set_Critical(FanInfo* o) {
	o->_set |= (1 << 3);
}

static inline void FanInfo_UnSet_Critical(FanInfo* o) {
	o->_set &= ~(1 << 3);
}

static inline bool FanInfo_IsSet_Critical(const FanInfo* o) {
	return o->_set & (1 << 3);
}

static inline void FanInfo_Set_CurrentSpeed(FanInfo* o) {
	o->_set |= (1 << 4);
}

static inline void FanInfo_UnSet_CurrentSpeed(FanInfo* o) {
	o->_set &= ~(1 << 4);
}

static inline bool FanInfo_IsSet_CurrentSpeed(const FanInfo* o) {
	return o->_set & (1 << 4);
}

static inline void FanInfo_Set_TargetSpeed(FanInfo* o) {
	o->_set |= (1 << 5);
}

static inline void FanInfo_UnSet_TargetSpeed(FanInfo* o) {
	o->_set &= ~(1 << 5);
}

static inline bool FanInfo_IsSet_TargetSpeed(const FanInfo* o) {
	return o->_set & (1 << 5);
}

static inline void FanInfo_Set_RequestedSpeed(FanInfo* o) {
	o->_set |= (1 << 6);
}

static inline void FanInfo_UnSet_RequestedSpeed(FanInfo* o) {
	o->_set &= ~(1 << 6);
}

static inline bool FanInfo_IsSet_RequestedSpeed(const FanInfo* o) {
	return o->_set & (1 << 6);
}

static inline void FanInfo_Set_SpeedSteps(FanInfo* o) {
	o->_set |= (1 << 7);
}

static inline void FanInfo_UnSet_SpeedSteps(FanInfo* o) {
	o->_set &= ~(1 << 7);
}

static inline bool FanInfo_IsSet_SpeedSteps(const FanInfo* o) {
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

static inline bool ServiceInfo_IsSet_PID(const ServiceInfo* o) {
	return o->_set & (1 << 0);
}

static inline void ServiceInfo_Set_SelectedConfigId(ServiceInfo* o) {
	o->_set |= (1 << 1);
}

static inline void ServiceInfo_UnSet_SelectedConfigId(ServiceInfo* o) {
	o->_set &= ~(1 << 1);
}

static inline bool ServiceInfo_IsSet_SelectedConfigId(const ServiceInfo* o) {
	return o->_set & (1 << 1);
}

static inline void ServiceInfo_Set_ReadOnly(ServiceInfo* o) {
	o->_set |= (1 << 2);
}

static inline void ServiceInfo_UnSet_ReadOnly(ServiceInfo* o) {
	o->_set &= ~(1 << 2);
}

static inline bool ServiceInfo_IsSet_ReadOnly(const ServiceInfo* o) {
	return o->_set & (1 << 2);
}

static inline void ServiceInfo_Set_Fans(ServiceInfo* o) {
	o->_set |= (1 << 3);
}

static inline void ServiceInfo_UnSet_Fans(ServiceInfo* o) {
	o->_set &= ~(1 << 3);
}

static inline bool ServiceInfo_IsSet_Fans(const ServiceInfo* o) {
	return o->_set & (1 << 3);
}

