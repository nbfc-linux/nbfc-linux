/* Auto generated code ['./tools/config.py', 'source'] */

struct TemperatureThreshold TemperatureThreshold_Unset = {
	short_Unset,
	short_Unset,
	float_Unset,
};

Error* TemperatureThreshold_ValidateFields(TemperatureThreshold* self) {
	if (self->UpThreshold == short_Unset)
		return err_string(0, "UpThreshold: Missing option");

	if (self->DownThreshold == short_Unset)
		return err_string(0, "DownThreshold: Missing option");

	if (self->FanSpeed == float_Unset)
		return err_string(0, "FanSpeed: Missing option");
	else if (! (self->FanSpeed >= 0.0 && self->FanSpeed <= 100.0))
		return err_string(0, "FanSpeed: requires: parameter >= 0.0 && parameter <= 100.0");
	return err_success();
}

Error* TemperatureThreshold_FromJson(TemperatureThreshold* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = TemperatureThreshold_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "not a JSON object");

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
		return err_string(0, "FanSpeedPercentage: Missing option");
	else if (! (self->FanSpeedPercentage >= 0.0 && self->FanSpeedPercentage <= 100.0))
		return err_string(0, "FanSpeedPercentage: requires: parameter >= 0.0 && parameter <= 100.0");

	if (self->FanSpeedValue == int_Unset)
		return err_string(0, "FanSpeedValue: Missing option");
	else if (! (self->FanSpeedValue >= 0 && self->FanSpeedValue <= 65535))
		return err_string(0, "FanSpeedValue: requires: parameter >= 0 && parameter <= 65535");

	if (self->TargetOperation == OverrideTargetOperation_Unset)
		self->TargetOperation = OverrideTargetOperation_ReadWrite;
	return err_success();
}

Error* FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = FanSpeedPercentageOverride_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "not a JSON object");

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
		return err_string(0, "Register: Missing option");
	else if (! (self->Register >= 0 && self->Register <= 255))
		return err_string(0, "Register: requires: parameter >= 0 && parameter <= 255");

	if (self->Value == int_Unset)
		return err_string(0, "Value: Missing option");
	else if (! (self->Value >= 0 && self->Value <= 65535))
		return err_string(0, "Value: requires: parameter >= 0 && parameter <= 65535");

	if (self->ResetRequired == Boolean_Unset)
		self->ResetRequired = Boolean_False;

	if (self->ResetValue == int_Unset)
		return err_string(0, "ResetValue: Missing option");
	else if (! (self->ResetValue >= 0 && self->ResetValue <= 65535))
		return err_string(0, "ResetValue: requires: parameter >= 0 && parameter <= 65535");

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
		return err_string(0, "not a JSON object");

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
	{NULL, 0},
	{NULL, 0},
};

Error* FanConfiguration_ValidateFields(FanConfiguration* self) {
	if (self->FanDisplayName == str_Unset)
		self->FanDisplayName = Mem_Strdup("");

	if (self->ReadRegister == short_Unset)
		return err_string(0, "ReadRegister: Missing option");
	else if (! (self->ReadRegister >= 0 && self->ReadRegister <= 255))
		return err_string(0, "ReadRegister: requires: parameter >= 0 && parameter <= 255");

	if (self->WriteRegister == short_Unset)
		return err_string(0, "WriteRegister: Missing option");
	else if (! (self->WriteRegister >= 0 && self->WriteRegister <= 255))
		return err_string(0, "WriteRegister: requires: parameter >= 0 && parameter <= 255");

	if (self->MinSpeedValue == int_Unset)
		return err_string(0, "MinSpeedValue: Missing option");
	else if (! (self->MinSpeedValue >= 0 && self->MinSpeedValue <= 65535))
		return err_string(0, "MinSpeedValue: requires: parameter >= 0 && parameter <= 65535");

	if (self->MaxSpeedValue == int_Unset)
		return err_string(0, "MaxSpeedValue: Missing option");
	else if (! (self->MaxSpeedValue >= 0 && self->MaxSpeedValue <= 65535))
		return err_string(0, "MaxSpeedValue: requires: parameter >= 0 && parameter <= 65535");

	if (self->MinSpeedValueRead == int_Unset)
		self->MinSpeedValueRead = 0;
	else if (! (self->MinSpeedValueRead >= 0 && self->MinSpeedValueRead <= 65535))
		return err_string(0, "MinSpeedValueRead: requires: parameter >= 0 && parameter <= 65535");

	if (self->MaxSpeedValueRead == int_Unset)
		self->MaxSpeedValueRead = 0;
	else if (! (self->MaxSpeedValueRead >= 0 && self->MaxSpeedValueRead <= 65535))
		return err_string(0, "MaxSpeedValueRead: requires: parameter >= 0 && parameter <= 65535");

	if (self->IndependentReadMinMaxValues == Boolean_Unset)
		self->IndependentReadMinMaxValues = Boolean_False;

	if (self->ResetRequired == Boolean_Unset)
		self->ResetRequired = Boolean_False;

	if (self->FanSpeedResetValue == int_Unset)
		self->FanSpeedResetValue = 0;
	else if (! (self->FanSpeedResetValue >= 0 && self->FanSpeedResetValue <= 65535))
		return err_string(0, "FanSpeedResetValue: requires: parameter >= 0 && parameter <= 65535");

	if (self->TemperatureThresholds.data == NULL)
		self->TemperatureThresholds = Config_DefaultTemperatureThresholds;

	if (self->FanSpeedPercentageOverrides.data == NULL)
		self->FanSpeedPercentageOverrides = Config_DefaultFanSpeedPercentageOverrides;
	return err_success();
}

Error* FanConfiguration_FromJson(FanConfiguration* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = FanConfiguration_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "not a JSON object");

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
	short_Unset,
	short_Unset,
	short_Unset,
	Boolean_Unset,
	{NULL, 0},
	{NULL, 0},
};

Error* ModelConfig_ValidateFields(ModelConfig* self) {
	if (self->NotebookModel == str_Unset)
		return err_string(0, "NotebookModel: Missing option");

	if (self->Author == str_Unset)
		self->Author = Mem_Strdup("");

	if (self->EcPollInterval == short_Unset)
		self->EcPollInterval = 3000;
	else if (! (self->EcPollInterval > 0))
		return err_string(0, "EcPollInterval: requires: parameter > 0");

	if (self->CriticalTemperature == short_Unset)
		self->CriticalTemperature = 75;

	if (self->CriticalTemperatureOffset == short_Unset)
		self->CriticalTemperatureOffset = 15;
	else if (! (self->CriticalTemperatureOffset > 0))
		return err_string(0, "CriticalTemperatureOffset: requires: parameter > 0");

	if (self->ReadWriteWords == Boolean_Unset)
		self->ReadWriteWords = Boolean_False;

	if (self->FanConfigurations.data == NULL)
		return err_string(0, "FanConfigurations: Missing option");
	else if (! (self->FanConfigurations.size > 0))
		return err_string(0, "FanConfigurations: requires: parameter.size > 0");

	if (false)
		return err_string(0, "RegisterWriteConfigurations: Missing option");
	return err_success();
}

Error* ModelConfig_FromJson(ModelConfig* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = ModelConfig_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "NotebookModel"))
			e = str_FromJson(&obj->NotebookModel, c);
		else if (!strcmp(c->key, "Author"))
			e = str_FromJson(&obj->Author, c);
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

struct ServiceConfig ServiceConfig_Unset = {
	str_Unset,
	Boolean_Unset,
	EmbeddedControllerType_Unset,
	{NULL, 0},
};

Error* ServiceConfig_ValidateFields(ServiceConfig* self) {
	if (self->SelectedConfigId == str_Unset)
		return err_string(0, "SelectedConfigId: Missing option");

	if (self->ReadOnly == Boolean_Unset)
		self->ReadOnly = Boolean_False;

	if (false)
		return err_string(0, "EmbeddedControllerType: Missing option");

	if (false)
		return err_string(0, "TargetFanSpeeds: Missing option");
	return err_success();
}

Error* ServiceConfig_FromJson(ServiceConfig* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = ServiceConfig_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "SelectedConfigId"))
			e = str_FromJson(&obj->SelectedConfigId, c);
		else if (!strcmp(c->key, "ReadOnly"))
			e = Boolean_FromJson(&obj->ReadOnly, c);
		else if (!strcmp(c->key, "EmbeddedControllerType"))
			e = EmbeddedControllerType_FromJson(&obj->EmbeddedControllerType, c);
		else if (!strcmp(c->key, "TargetFanSpeeds"))
			e = array_of_float_FromJson(&obj->TargetFanSpeeds, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

struct FanInfo FanInfo_Unset = {
	str_Unset,
	Boolean_Unset,
	Boolean_Unset,
	float_Unset,
	float_Unset,
	int_Unset,
};

Error* FanInfo_ValidateFields(FanInfo* self) {
	if (self->name == str_Unset)
		return err_string(0, "name: Missing option");

	if (self->automode == Boolean_Unset)
		return err_string(0, "automode: Missing option");

	if (self->critical == Boolean_Unset)
		return err_string(0, "critical: Missing option");

	if (self->current_speed == float_Unset)
		return err_string(0, "current_speed: Missing option");

	if (self->target_speed == float_Unset)
		return err_string(0, "target_speed: Missing option");

	if (self->speed_steps == int_Unset)
		return err_string(0, "speed_steps: Missing option");
	return err_success();
}

Error* FanInfo_FromJson(FanInfo* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = FanInfo_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "name"))
			e = str_FromJson(&obj->name, c);
		else if (!strcmp(c->key, "automode"))
			e = Boolean_FromJson(&obj->automode, c);
		else if (!strcmp(c->key, "critical"))
			e = Boolean_FromJson(&obj->critical, c);
		else if (!strcmp(c->key, "current_speed"))
			e = float_FromJson(&obj->current_speed, c);
		else if (!strcmp(c->key, "target_speed"))
			e = float_FromJson(&obj->target_speed, c);
		else if (!strcmp(c->key, "speed_steps"))
			e = int_FromJson(&obj->speed_steps, c);
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
	float_Unset,
	{NULL, 0},
};

Error* ServiceInfo_ValidateFields(ServiceInfo* self) {
	if (self->pid == int_Unset)
		return err_string(0, "pid: Missing option");

	if (self->config == str_Unset)
		return err_string(0, "config: Missing option");

	if (self->readonly == Boolean_Unset)
		return err_string(0, "readonly: Missing option");

	if (self->temperature == float_Unset)
		return err_string(0, "temperature: Missing option");

	if (self->fans.data == NULL)
		return err_string(0, "fans: Missing option");
	return err_success();
}

Error* ServiceInfo_FromJson(ServiceInfo* obj, const nx_json* json) {
	Error* e = NULL;
	*obj = ServiceInfo_Unset;

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "not a JSON object");

	nx_json_for_each(c, json) {
		if (0);
		else if (!strcmp(c->key, "pid"))
			e = int_FromJson(&obj->pid, c);
		else if (!strcmp(c->key, "config"))
			e = str_FromJson(&obj->config, c);
		else if (!strcmp(c->key, "readonly"))
			e = Boolean_FromJson(&obj->readonly, c);
		else if (!strcmp(c->key, "temperature"))
			e = float_FromJson(&obj->temperature, c);
		else if (!strcmp(c->key, "fans"))
			e = array_of_FanInfo_FromJson(&obj->fans, c);
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

