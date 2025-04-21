/* Auto generated code ['./tools/config.py', 'source'] */

struct TemperatureThreshold TemperatureThreshold_Unset = {
	short_Unset,
	short_Unset,
	float_Unset,
};

Error* TemperatureThreshold_ValidateFields(TemperatureThreshold* self) {
	if (self->UpThreshold == short_Unset)
		return err_stringf(0, "%s: %s", "UpThreshold", "Missing option");

	if (self->DownThreshold == short_Unset)
		return err_stringf(0, "%s: %s", "DownThreshold", "Missing option");

	if (self->FanSpeed == float_Unset)
		return err_stringf(0, "%s: %s", "FanSpeed", "Missing option");
	else if (! (self->FanSpeed >= 0.0 && self->FanSpeed <= 100.0))
		return err_stringf(0, "%s: %s", "FanSpeed", "requires: parameter >= 0.0 && parameter <= 100.0");
	return err_success();
}

Error* TemperatureThreshold_FromJson(TemperatureThreshold* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = TemperatureThreshold_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "UpThreshold"))
			e = short_FromJson(&obj->UpThreshold, c);
		else if (!strcmp(c->key, "DownThreshold"))
			e = short_FromJson(&obj->DownThreshold, c);
		else if (!strcmp(c->key, "FanSpeed"))
			e = float_FromJson(&obj->FanSpeed, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct FanSpeedPercentageOverride FanSpeedPercentageOverride_Unset = {
	float_Unset,
	int_Unset,
	OverrideTargetOperation_Unset,
};

Error* FanSpeedPercentageOverride_ValidateFields(FanSpeedPercentageOverride* self) {
	if (self->FanSpeedPercentage == float_Unset)
		return err_stringf(0, "%s: %s", "FanSpeedPercentage", "Missing option");
	else if (! (self->FanSpeedPercentage >= 0.0 && self->FanSpeedPercentage <= 100.0))
		return err_stringf(0, "%s: %s", "FanSpeedPercentage", "requires: parameter >= 0.0 && parameter <= 100.0");

	if (self->FanSpeedValue == int_Unset)
		return err_stringf(0, "%s: %s", "FanSpeedValue", "Missing option");
	else if (! (self->FanSpeedValue >= 0 && self->FanSpeedValue <= 65535))
		return err_stringf(0, "%s: %s", "FanSpeedValue", "requires: parameter >= 0 && parameter <= 65535");

	if (self->TargetOperation == OverrideTargetOperation_Unset)
		self->TargetOperation = OverrideTargetOperation_ReadWrite;
	return err_success();
}

Error* FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = FanSpeedPercentageOverride_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "FanSpeedPercentage"))
			e = float_FromJson(&obj->FanSpeedPercentage, c);
		else if (!strcmp(c->key, "FanSpeedValue"))
			e = int_FromJson(&obj->FanSpeedValue, c);
		else if (!strcmp(c->key, "TargetOperation"))
			e = OverrideTargetOperation_FromJson(&obj->TargetOperation, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct RegisterWriteConfiguration RegisterWriteConfiguration_Unset = {
	RegisterWriteMode_Unset,
	RegisterWriteOccasion_Unset,
	short_Unset,
	int_Unset,
	Boolean_Unset,
	int_Unset,
	RegisterWriteMode_Unset,
	str_Unset,
};

Error* RegisterWriteConfiguration_ValidateFields(RegisterWriteConfiguration* self) {
	if (self->WriteMode == RegisterWriteMode_Unset)
		self->WriteMode = RegisterWriteMode_Set;

	if (self->WriteOccasion == RegisterWriteOccasion_Unset)
		self->WriteOccasion = RegisterWriteOccasion_OnInitialization;

	if (self->Register == short_Unset)
		return err_stringf(0, "%s: %s", "Register", "Missing option");
	else if (! (self->Register >= 0 && self->Register <= 255))
		return err_stringf(0, "%s: %s", "Register", "requires: parameter >= 0 && parameter <= 255");

	if (self->Value == int_Unset)
		return err_stringf(0, "%s: %s", "Value", "Missing option");
	else if (! (self->Value >= 0 && self->Value <= 65535))
		return err_stringf(0, "%s: %s", "Value", "requires: parameter >= 0 && parameter <= 65535");

	if (self->ResetRequired == Boolean_Unset)
		self->ResetRequired = Boolean_False;

	if (self->ResetValue == int_Unset)
		return err_stringf(0, "%s: %s", "ResetValue", "Missing option");
	else if (! (self->ResetValue >= 0 && self->ResetValue <= 65535))
		return err_stringf(0, "%s: %s", "ResetValue", "requires: parameter >= 0 && parameter <= 65535");

	if (self->ResetWriteMode == RegisterWriteMode_Unset)
		self->ResetWriteMode = RegisterWriteMode_Set;

	if (self->Description == str_Unset)
		self->Description = Mem_Strdup("");
	return err_success();
}

Error* RegisterWriteConfiguration_FromJson(RegisterWriteConfiguration* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = RegisterWriteConfiguration_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "WriteMode"))
			e = RegisterWriteMode_FromJson(&obj->WriteMode, c);
		else if (!strcmp(c->key, "WriteOccasion"))
			e = RegisterWriteOccasion_FromJson(&obj->WriteOccasion, c);
		else if (!strcmp(c->key, "Register"))
			e = short_FromJson(&obj->Register, c);
		else if (!strcmp(c->key, "Value"))
			e = int_FromJson(&obj->Value, c);
		else if (!strcmp(c->key, "ResetRequired"))
			e = Boolean_FromJson(&obj->ResetRequired, c);
		else if (!strcmp(c->key, "ResetValue"))
			e = int_FromJson(&obj->ResetValue, c);
		else if (!strcmp(c->key, "ResetWriteMode"))
			e = RegisterWriteMode_FromJson(&obj->ResetWriteMode, c);
		else if (!strcmp(c->key, "Description"))
			e = str_FromJson(&obj->Description, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct FanConfiguration FanConfiguration_Unset = {
	str_Unset,
	short_Unset,
	short_Unset,
	int_Unset,
	int_Unset,
	int_Unset,
	int_Unset,
	Boolean_Unset,
	Boolean_Unset,
	int_Unset,
	TemperatureAlgorithmType_Unset,
	{NULL, 0},
	{NULL, 0},
	{NULL, 0},
};

Error* FanConfiguration_ValidateFields(FanConfiguration* self) {
	if (false)
		return err_stringf(0, "%s: %s", "FanDisplayName", "Missing option");

	if (self->ReadRegister == short_Unset)
		return err_stringf(0, "%s: %s", "ReadRegister", "Missing option");
	else if (! (self->ReadRegister >= 0 && self->ReadRegister <= 255))
		return err_stringf(0, "%s: %s", "ReadRegister", "requires: parameter >= 0 && parameter <= 255");

	if (self->WriteRegister == short_Unset)
		return err_stringf(0, "%s: %s", "WriteRegister", "Missing option");
	else if (! (self->WriteRegister >= 0 && self->WriteRegister <= 255))
		return err_stringf(0, "%s: %s", "WriteRegister", "requires: parameter >= 0 && parameter <= 255");

	if (self->MinSpeedValue == int_Unset)
		return err_stringf(0, "%s: %s", "MinSpeedValue", "Missing option");
	else if (! (self->MinSpeedValue >= 0 && self->MinSpeedValue <= 65535))
		return err_stringf(0, "%s: %s", "MinSpeedValue", "requires: parameter >= 0 && parameter <= 65535");

	if (self->MaxSpeedValue == int_Unset)
		return err_stringf(0, "%s: %s", "MaxSpeedValue", "Missing option");
	else if (! (self->MaxSpeedValue >= 0 && self->MaxSpeedValue <= 65535))
		return err_stringf(0, "%s: %s", "MaxSpeedValue", "requires: parameter >= 0 && parameter <= 65535");

	if (self->MinSpeedValueRead == int_Unset)
		self->MinSpeedValueRead = 0;
	else if (! (self->MinSpeedValueRead >= 0 && self->MinSpeedValueRead <= 65535))
		return err_stringf(0, "%s: %s", "MinSpeedValueRead", "requires: parameter >= 0 && parameter <= 65535");

	if (self->MaxSpeedValueRead == int_Unset)
		self->MaxSpeedValueRead = 0;
	else if (! (self->MaxSpeedValueRead >= 0 && self->MaxSpeedValueRead <= 65535))
		return err_stringf(0, "%s: %s", "MaxSpeedValueRead", "requires: parameter >= 0 && parameter <= 65535");

	if (self->IndependentReadMinMaxValues == Boolean_Unset)
		self->IndependentReadMinMaxValues = Boolean_False;

	if (self->ResetRequired == Boolean_Unset)
		self->ResetRequired = Boolean_False;

	if (self->FanSpeedResetValue == int_Unset)
		self->FanSpeedResetValue = 0;
	else if (! (self->FanSpeedResetValue >= 0 && self->FanSpeedResetValue <= 65535))
		return err_stringf(0, "%s: %s", "FanSpeedResetValue", "requires: parameter >= 0 && parameter <= 65535");

	if (false)
		return err_stringf(0, "%s: %s", "TemperatureAlgorithmType", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "Sensors", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "TemperatureThresholds", "Missing option");

	if (self->FanSpeedPercentageOverrides.data == NULL)
		self->FanSpeedPercentageOverrides = Config_DefaultFanSpeedPercentageOverrides;
	return err_success();
}

Error* FanConfiguration_FromJson(FanConfiguration* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = FanConfiguration_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "FanDisplayName"))
			e = str_FromJson(&obj->FanDisplayName, c);
		else if (!strcmp(c->key, "ReadRegister"))
			e = short_FromJson(&obj->ReadRegister, c);
		else if (!strcmp(c->key, "WriteRegister"))
			e = short_FromJson(&obj->WriteRegister, c);
		else if (!strcmp(c->key, "MinSpeedValue"))
			e = int_FromJson(&obj->MinSpeedValue, c);
		else if (!strcmp(c->key, "MaxSpeedValue"))
			e = int_FromJson(&obj->MaxSpeedValue, c);
		else if (!strcmp(c->key, "MinSpeedValueRead"))
			e = int_FromJson(&obj->MinSpeedValueRead, c);
		else if (!strcmp(c->key, "MaxSpeedValueRead"))
			e = int_FromJson(&obj->MaxSpeedValueRead, c);
		else if (!strcmp(c->key, "IndependentReadMinMaxValues"))
			e = Boolean_FromJson(&obj->IndependentReadMinMaxValues, c);
		else if (!strcmp(c->key, "ResetRequired"))
			e = Boolean_FromJson(&obj->ResetRequired, c);
		else if (!strcmp(c->key, "FanSpeedResetValue"))
			e = int_FromJson(&obj->FanSpeedResetValue, c);
		else if (!strcmp(c->key, "TemperatureAlgorithmType"))
			e = TemperatureAlgorithmType_FromJson(&obj->TemperatureAlgorithmType, c);
		else if (!strcmp(c->key, "Sensors"))
			e = array_of_str_FromJson(&obj->Sensors, c);
		else if (!strcmp(c->key, "TemperatureThresholds"))
			e = array_of_TemperatureThreshold_FromJson(&obj->TemperatureThresholds, c);
		else if (!strcmp(c->key, "FanSpeedPercentageOverrides"))
			e = array_of_FanSpeedPercentageOverride_FromJson(&obj->FanSpeedPercentageOverrides, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct ModelConfig ModelConfig_Unset = {
	str_Unset,
	str_Unset,
	Boolean_Unset,
	short_Unset,
	short_Unset,
	short_Unset,
	Boolean_Unset,
	{NULL, 0},
	{NULL, 0},
};

Error* ModelConfig_ValidateFields(ModelConfig* self) {
	if (self->NotebookModel == str_Unset)
		return err_stringf(0, "%s: %s", "NotebookModel", "Missing option");

	if (self->Author == str_Unset)
		self->Author = Mem_Strdup("");

	if (self->LegacyTemperatureThresholdsBehaviour == Boolean_Unset)
		self->LegacyTemperatureThresholdsBehaviour = Boolean_False;

	if (self->EcPollInterval == short_Unset)
		self->EcPollInterval = 3000;
	else if (! (self->EcPollInterval > 0))
		return err_stringf(0, "%s: %s", "EcPollInterval", "requires: parameter > 0");

	if (self->CriticalTemperature == short_Unset)
		self->CriticalTemperature = 75;

	if (self->CriticalTemperatureOffset == short_Unset)
		self->CriticalTemperatureOffset = 15;
	else if (! (self->CriticalTemperatureOffset > 0))
		return err_stringf(0, "%s: %s", "CriticalTemperatureOffset", "requires: parameter > 0");

	if (self->ReadWriteWords == Boolean_Unset)
		self->ReadWriteWords = Boolean_False;

	if (self->FanConfigurations.data == NULL)
		return err_stringf(0, "%s: %s", "FanConfigurations", "Missing option");
	else if (! (self->FanConfigurations.size > 0))
		return err_stringf(0, "%s: %s", "FanConfigurations", "requires: parameter.size > 0");

	if (false)
		return err_stringf(0, "%s: %s", "RegisterWriteConfigurations", "Missing option");
	return err_success();
}

Error* ModelConfig_FromJson(ModelConfig* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = ModelConfig_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "NotebookModel"))
			e = str_FromJson(&obj->NotebookModel, c);
		else if (!strcmp(c->key, "Author"))
			e = str_FromJson(&obj->Author, c);
		else if (!strcmp(c->key, "LegacyTemperatureThresholdsBehaviour"))
			e = Boolean_FromJson(&obj->LegacyTemperatureThresholdsBehaviour, c);
		else if (!strcmp(c->key, "EcPollInterval"))
			e = short_FromJson(&obj->EcPollInterval, c);
		else if (!strcmp(c->key, "CriticalTemperature"))
			e = short_FromJson(&obj->CriticalTemperature, c);
		else if (!strcmp(c->key, "CriticalTemperatureOffset"))
			e = short_FromJson(&obj->CriticalTemperatureOffset, c);
		else if (!strcmp(c->key, "ReadWriteWords"))
			e = Boolean_FromJson(&obj->ReadWriteWords, c);
		else if (!strcmp(c->key, "FanConfigurations"))
			e = array_of_FanConfiguration_FromJson(&obj->FanConfigurations, c);
		else if (!strcmp(c->key, "RegisterWriteConfigurations"))
			e = array_of_RegisterWriteConfiguration_FromJson(&obj->RegisterWriteConfigurations, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct FanTemperatureSourceConfig FanTemperatureSourceConfig_Unset = {
	int_Unset,
	TemperatureAlgorithmType_Unset,
	{NULL, 0},
};

Error* FanTemperatureSourceConfig_ValidateFields(FanTemperatureSourceConfig* self) {
	if (self->FanIndex == int_Unset)
		return err_stringf(0, "%s: %s", "FanIndex", "Missing option");

	if (self->TemperatureAlgorithmType == TemperatureAlgorithmType_Unset)
		self->TemperatureAlgorithmType = TemperatureAlgorithmType_Average;

	if (false)
		return err_stringf(0, "%s: %s", "Sensors", "Missing option");
	return err_success();
}

Error* FanTemperatureSourceConfig_FromJson(FanTemperatureSourceConfig* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = FanTemperatureSourceConfig_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "FanIndex"))
			e = int_FromJson(&obj->FanIndex, c);
		else if (!strcmp(c->key, "TemperatureAlgorithmType"))
			e = TemperatureAlgorithmType_FromJson(&obj->TemperatureAlgorithmType, c);
		else if (!strcmp(c->key, "Sensors"))
			e = array_of_str_FromJson(&obj->Sensors, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct ServiceConfig ServiceConfig_Unset = {
	str_Unset,
	EmbeddedControllerType_Unset,
	{NULL, 0},
	{NULL, 0},
};

Error* ServiceConfig_ValidateFields(ServiceConfig* self) {
	if (self->SelectedConfigId == str_Unset)
		return err_stringf(0, "%s: %s", "SelectedConfigId", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "EmbeddedControllerType", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "TargetFanSpeeds", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "FanTemperatureSources", "Missing option");
	return err_success();
}

Error* ServiceConfig_FromJson(ServiceConfig* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = ServiceConfig_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "SelectedConfigId"))
			e = str_FromJson(&obj->SelectedConfigId, c);
		else if (!strcmp(c->key, "EmbeddedControllerType"))
			e = EmbeddedControllerType_FromJson(&obj->EmbeddedControllerType, c);
		else if (!strcmp(c->key, "TargetFanSpeeds"))
			e = array_of_float_FromJson(&obj->TargetFanSpeeds, c);
		else if (!strcmp(c->key, "FanTemperatureSources"))
			e = array_of_FanTemperatureSourceConfig_FromJson(&obj->FanTemperatureSources, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct FanInfo FanInfo_Unset = {
	str_Unset,
	float_Unset,
	Boolean_Unset,
	Boolean_Unset,
	float_Unset,
	float_Unset,
	float_Unset,
	int_Unset,
};

Error* FanInfo_ValidateFields(FanInfo* self) {
	if (self->Name == str_Unset)
		return err_stringf(0, "%s: %s", "Name", "Missing option");

	if (self->Temperature == float_Unset)
		return err_stringf(0, "%s: %s", "Temperature", "Missing option");

	if (self->AutoMode == Boolean_Unset)
		return err_stringf(0, "%s: %s", "AutoMode", "Missing option");

	if (self->Critical == Boolean_Unset)
		return err_stringf(0, "%s: %s", "Critical", "Missing option");

	if (self->CurrentSpeed == float_Unset)
		return err_stringf(0, "%s: %s", "CurrentSpeed", "Missing option");

	if (self->TargetSpeed == float_Unset)
		return err_stringf(0, "%s: %s", "TargetSpeed", "Missing option");

	if (self->RequestedSpeed == float_Unset)
		return err_stringf(0, "%s: %s", "RequestedSpeed", "Missing option");

	if (self->SpeedSteps == int_Unset)
		return err_stringf(0, "%s: %s", "SpeedSteps", "Missing option");
	return err_success();
}

Error* FanInfo_FromJson(FanInfo* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = FanInfo_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "Name"))
			e = str_FromJson(&obj->Name, c);
		else if (!strcmp(c->key, "Temperature"))
			e = float_FromJson(&obj->Temperature, c);
		else if (!strcmp(c->key, "AutoMode"))
			e = Boolean_FromJson(&obj->AutoMode, c);
		else if (!strcmp(c->key, "Critical"))
			e = Boolean_FromJson(&obj->Critical, c);
		else if (!strcmp(c->key, "CurrentSpeed"))
			e = float_FromJson(&obj->CurrentSpeed, c);
		else if (!strcmp(c->key, "TargetSpeed"))
			e = float_FromJson(&obj->TargetSpeed, c);
		else if (!strcmp(c->key, "RequestedSpeed"))
			e = float_FromJson(&obj->RequestedSpeed, c);
		else if (!strcmp(c->key, "SpeedSteps"))
			e = int_FromJson(&obj->SpeedSteps, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct ServiceInfo ServiceInfo_Unset = {
	int_Unset,
	str_Unset,
	Boolean_Unset,
	{NULL, 0},
};

Error* ServiceInfo_ValidateFields(ServiceInfo* self) {
	if (self->PID == int_Unset)
		return err_stringf(0, "%s: %s", "PID", "Missing option");

	if (self->SelectedConfigId == str_Unset)
		return err_stringf(0, "%s: %s", "SelectedConfigId", "Missing option");

	if (self->ReadOnly == Boolean_Unset)
		return err_stringf(0, "%s: %s", "ReadOnly", "Missing option");

	if (self->Fans.data == NULL)
		return err_stringf(0, "%s: %s", "Fans", "Missing option");
	return err_success();
}

Error* ServiceInfo_FromJson(ServiceInfo* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = ServiceInfo_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "PID"))
			e = int_FromJson(&obj->PID, c);
		else if (!strcmp(c->key, "SelectedConfigId"))
			e = str_FromJson(&obj->SelectedConfigId, c);
		else if (!strcmp(c->key, "ReadOnly"))
			e = Boolean_FromJson(&obj->ReadOnly, c);
		else if (!strcmp(c->key, "Fans"))
			e = array_of_FanInfo_FromJson(&obj->Fans, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

