/* Auto generated code ['./tools/config.py', 'source'] */

Error TemperatureThreshold_ValidateFields(TemperatureThreshold* self) {
	(void) self;

	if (! self->isset.UpThreshold)
		return err_stringf("%s: %s", "UpThreshold", "Missing option");

	if (! self->isset.DownThreshold)
		return err_stringf("%s: %s", "DownThreshold", "Missing option");

	if (! self->isset.FanSpeed)
		return err_stringf("%s: %s", "FanSpeed", "Missing option");
	else if (! (self->FanSpeed >= 0.0f && self->FanSpeed <= 100.0f))
		return err_stringf("%s: %s", "FanSpeed", "requires: 0.0 <= parameter <= 100.0");
	return err_success();
}

Error TemperatureThreshold_FromJson(TemperatureThreshold* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "UpThreshold")) {
			e = int16_t_FromJson(&obj->UpThreshold, c);
			if (!e)
				obj->isset.UpThreshold = true;
		}
		else if (!strcmp(c->key, "DownThreshold")) {
			e = int16_t_FromJson(&obj->DownThreshold, c);
			if (!e)
				obj->isset.DownThreshold = true;
		}
		else if (!strcmp(c->key, "FanSpeed")) {
			e = float_FromJson(&obj->FanSpeed, c);
			if (!e)
				obj->isset.FanSpeed = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error FanSpeedPercentageOverride_ValidateFields(FanSpeedPercentageOverride* self) {
	(void) self;

	if (! self->isset.FanSpeedPercentage)
		return err_stringf("%s: %s", "FanSpeedPercentage", "Missing option");
	else if (! (self->FanSpeedPercentage >= 0.0f && self->FanSpeedPercentage <= 100.0f))
		return err_stringf("%s: %s", "FanSpeedPercentage", "requires: 0.0 <= parameter <= 100.0");

	if (! self->isset.FanSpeedValue)
		return err_stringf("%s: %s", "FanSpeedValue", "Missing option");

	if (! self->isset.TargetOperation)
		self->TargetOperation = OverrideTargetOperation_ReadWrite;
	return err_success();
}

Error FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "FanSpeedPercentage")) {
			e = float_FromJson(&obj->FanSpeedPercentage, c);
			if (!e)
				obj->isset.FanSpeedPercentage = true;
		}
		else if (!strcmp(c->key, "FanSpeedValue")) {
			e = uint16_t_FromJson(&obj->FanSpeedValue, c);
			if (!e)
				obj->isset.FanSpeedValue = true;
		}
		else if (!strcmp(c->key, "TargetOperation")) {
			e = OverrideTargetOperation_FromJson(&obj->TargetOperation, c);
			if (!e)
				obj->isset.TargetOperation = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error RegisterWriteConfiguration_ValidateFields(RegisterWriteConfiguration* self) {
	(void) self;

	if (! self->isset.WriteMode)
		self->WriteMode = RegisterWriteMode_Set;

	if (! self->isset.WriteOccasion)
		self->WriteOccasion = RegisterWriteOccasion_OnInitialization;

	if (false)
		return err_stringf("%s: %s", "Register", "Missing option");

	if (false)
		return err_stringf("%s: %s", "Value", "Missing option");

	if (false)
		return err_stringf("%s: %s", "AcpiMethod", "Missing option");

	if (false)
		return err_stringf("%s: %s", "LuaCode", "Missing option");

	if (! self->isset.ResetRequired)
		self->ResetRequired = false;

	if (false)
		return err_stringf("%s: %s", "ResetValue", "Missing option");

	if (false)
		return err_stringf("%s: %s", "ResetAcpiMethod", "Missing option");

	if (false)
		return err_stringf("%s: %s", "ResetLuaCode", "Missing option");

	if (! self->isset.ResetWriteMode)
		self->ResetWriteMode = RegisterWriteMode_Set;

	if (false)
		return err_stringf("%s: %s", "Description", "Missing option");
	return err_success();
}

Error RegisterWriteConfiguration_FromJson(RegisterWriteConfiguration* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "WriteMode")) {
			e = RegisterWriteMode_FromJson(&obj->WriteMode, c);
			if (!e)
				obj->isset.WriteMode = true;
		}
		else if (!strcmp(c->key, "WriteOccasion")) {
			e = RegisterWriteOccasion_FromJson(&obj->WriteOccasion, c);
			if (!e)
				obj->isset.WriteOccasion = true;
		}
		else if (!strcmp(c->key, "Register")) {
			e = uint8_t_FromJson(&obj->Register, c);
			if (!e)
				obj->isset.Register = true;
		}
		else if (!strcmp(c->key, "Value")) {
			e = uint8_t_FromJson(&obj->Value, c);
			if (!e)
				obj->isset.Value = true;
		}
		else if (!strcmp(c->key, "AcpiMethod")) {
			e = str_FromJson(&obj->AcpiMethod, c);
			if (!e)
				obj->isset.AcpiMethod = true;
		}
		else if (!strcmp(c->key, "LuaCode")) {
			e = LuaCode_FromJson(&obj->LuaCode, c);
			if (!e)
				obj->isset.LuaCode = true;
		}
		else if (!strcmp(c->key, "ResetRequired")) {
			e = bool_FromJson(&obj->ResetRequired, c);
			if (!e)
				obj->isset.ResetRequired = true;
		}
		else if (!strcmp(c->key, "ResetValue")) {
			e = uint8_t_FromJson(&obj->ResetValue, c);
			if (!e)
				obj->isset.ResetValue = true;
		}
		else if (!strcmp(c->key, "ResetAcpiMethod")) {
			e = str_FromJson(&obj->ResetAcpiMethod, c);
			if (!e)
				obj->isset.ResetAcpiMethod = true;
		}
		else if (!strcmp(c->key, "ResetLuaCode")) {
			e = LuaCode_FromJson(&obj->ResetLuaCode, c);
			if (!e)
				obj->isset.ResetLuaCode = true;
		}
		else if (!strcmp(c->key, "ResetWriteMode")) {
			e = RegisterWriteMode_FromJson(&obj->ResetWriteMode, c);
			if (!e)
				obj->isset.ResetWriteMode = true;
		}
		else if (!strcmp(c->key, "Description")) {
			e = str_FromJson(&obj->Description, c);
			if (!e)
				obj->isset.Description = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error FanConfiguration_ValidateFields(FanConfiguration* self) {
	(void) self;

	if (false)
		return err_stringf("%s: %s", "FanDisplayName", "Missing option");

	if (false)
		return err_stringf("%s: %s", "ReadRegister", "Missing option");

	if (false)
		return err_stringf("%s: %s", "ReadAcpiMethod", "Missing option");

	if (false)
		return err_stringf("%s: %s", "ReadLuaCode", "Missing option");

	if (false)
		return err_stringf("%s: %s", "WriteRegister", "Missing option");

	if (false)
		return err_stringf("%s: %s", "WriteAcpiMethod", "Missing option");

	if (false)
		return err_stringf("%s: %s", "WriteLuaCode", "Missing option");

	if (! self->isset.MinSpeedValue)
		return err_stringf("%s: %s", "MinSpeedValue", "Missing option");

	if (! self->isset.MaxSpeedValue)
		return err_stringf("%s: %s", "MaxSpeedValue", "Missing option");

	if (false)
		return err_stringf("%s: %s", "MinSpeedValueRead", "Missing option");

	if (false)
		return err_stringf("%s: %s", "MaxSpeedValueRead", "Missing option");

	if (! self->isset.IndependentReadMinMaxValues)
		self->IndependentReadMinMaxValues = false;

	if (! self->isset.ResetRequired)
		self->ResetRequired = false;

	if (false)
		return err_stringf("%s: %s", "FanSpeedResetValue", "Missing option");

	if (false)
		return err_stringf("%s: %s", "ResetAcpiMethod", "Missing option");

	if (false)
		return err_stringf("%s: %s", "ResetLuaCode", "Missing option");

	if (false)
		return err_stringf("%s: %s", "TemperatureAlgorithmType", "Missing option");

	if (false)
		return err_stringf("%s: %s", "Sensors", "Missing option");

	if (false)
		return err_stringf("%s: %s", "TemperatureThresholds", "Missing option");

	if (! self->isset.FanSpeedPercentageOverrides)
		self->FanSpeedPercentageOverrides = Config_DefaultFanSpeedPercentageOverrides;
	return err_success();
}

Error FanConfiguration_FromJson(FanConfiguration* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "FanDisplayName")) {
			e = str_FromJson(&obj->FanDisplayName, c);
			if (!e)
				obj->isset.FanDisplayName = true;
		}
		else if (!strcmp(c->key, "ReadRegister")) {
			e = uint8_t_FromJson(&obj->ReadRegister, c);
			if (!e)
				obj->isset.ReadRegister = true;
		}
		else if (!strcmp(c->key, "ReadAcpiMethod")) {
			e = str_FromJson(&obj->ReadAcpiMethod, c);
			if (!e)
				obj->isset.ReadAcpiMethod = true;
		}
		else if (!strcmp(c->key, "ReadLuaCode")) {
			e = LuaCode_FromJson(&obj->ReadLuaCode, c);
			if (!e)
				obj->isset.ReadLuaCode = true;
		}
		else if (!strcmp(c->key, "WriteRegister")) {
			e = uint8_t_FromJson(&obj->WriteRegister, c);
			if (!e)
				obj->isset.WriteRegister = true;
		}
		else if (!strcmp(c->key, "WriteAcpiMethod")) {
			e = str_FromJson(&obj->WriteAcpiMethod, c);
			if (!e)
				obj->isset.WriteAcpiMethod = true;
		}
		else if (!strcmp(c->key, "WriteLuaCode")) {
			e = LuaCode_FromJson(&obj->WriteLuaCode, c);
			if (!e)
				obj->isset.WriteLuaCode = true;
		}
		else if (!strcmp(c->key, "MinSpeedValue")) {
			e = uint16_t_FromJson(&obj->MinSpeedValue, c);
			if (!e)
				obj->isset.MinSpeedValue = true;
		}
		else if (!strcmp(c->key, "MaxSpeedValue")) {
			e = uint16_t_FromJson(&obj->MaxSpeedValue, c);
			if (!e)
				obj->isset.MaxSpeedValue = true;
		}
		else if (!strcmp(c->key, "MinSpeedValueRead")) {
			e = uint16_t_FromJson(&obj->MinSpeedValueRead, c);
			if (!e)
				obj->isset.MinSpeedValueRead = true;
		}
		else if (!strcmp(c->key, "MaxSpeedValueRead")) {
			e = uint16_t_FromJson(&obj->MaxSpeedValueRead, c);
			if (!e)
				obj->isset.MaxSpeedValueRead = true;
		}
		else if (!strcmp(c->key, "IndependentReadMinMaxValues")) {
			e = bool_FromJson(&obj->IndependentReadMinMaxValues, c);
			if (!e)
				obj->isset.IndependentReadMinMaxValues = true;
		}
		else if (!strcmp(c->key, "ResetRequired")) {
			e = bool_FromJson(&obj->ResetRequired, c);
			if (!e)
				obj->isset.ResetRequired = true;
		}
		else if (!strcmp(c->key, "FanSpeedResetValue")) {
			e = uint16_t_FromJson(&obj->FanSpeedResetValue, c);
			if (!e)
				obj->isset.FanSpeedResetValue = true;
		}
		else if (!strcmp(c->key, "ResetAcpiMethod")) {
			e = str_FromJson(&obj->ResetAcpiMethod, c);
			if (!e)
				obj->isset.ResetAcpiMethod = true;
		}
		else if (!strcmp(c->key, "ResetLuaCode")) {
			e = LuaCode_FromJson(&obj->ResetLuaCode, c);
			if (!e)
				obj->isset.ResetLuaCode = true;
		}
		else if (!strcmp(c->key, "TemperatureAlgorithmType")) {
			e = TemperatureAlgorithmType_FromJson(&obj->TemperatureAlgorithmType, c);
			if (!e)
				obj->isset.TemperatureAlgorithmType = true;
		}
		else if (!strcmp(c->key, "Sensors")) {
			e = array_of_str_FromJson(&obj->Sensors, c);
			if (!e)
				obj->isset.Sensors = true;
		}
		else if (!strcmp(c->key, "TemperatureThresholds")) {
			e = array_of_TemperatureThreshold_FromJson(&obj->TemperatureThresholds, c);
			if (!e)
				obj->isset.TemperatureThresholds = true;
		}
		else if (!strcmp(c->key, "FanSpeedPercentageOverrides")) {
			e = array_of_FanSpeedPercentageOverride_FromJson(&obj->FanSpeedPercentageOverrides, c);
			if (!e)
				obj->isset.FanSpeedPercentageOverrides = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error Sponsor_ValidateFields(Sponsor* self) {
	(void) self;

	if (! self->isset.Name)
		return err_stringf("%s: %s", "Name", "Missing option");

	if (false)
		return err_stringf("%s: %s", "Description", "Missing option");

	if (! self->isset.URL)
		return err_stringf("%s: %s", "URL", "Missing option");

	if (! self->isset.BannerURL)
		return err_stringf("%s: %s", "BannerURL", "Missing option");
	return err_success();
}

Error Sponsor_FromJson(Sponsor* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "Name")) {
			e = str_FromJson(&obj->Name, c);
			if (!e)
				obj->isset.Name = true;
		}
		else if (!strcmp(c->key, "Description")) {
			e = str_FromJson(&obj->Description, c);
			if (!e)
				obj->isset.Description = true;
		}
		else if (!strcmp(c->key, "URL")) {
			e = str_FromJson(&obj->URL, c);
			if (!e)
				obj->isset.URL = true;
		}
		else if (!strcmp(c->key, "BannerURL")) {
			e = str_FromJson(&obj->BannerURL, c);
			if (!e)
				obj->isset.BannerURL = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error ModelConfig_ValidateFields(ModelConfig* self) {
	(void) self;

	if (! self->isset.NotebookModel)
		return err_stringf("%s: %s", "NotebookModel", "Missing option");

	if (false)
		return err_stringf("%s: %s", "Author", "Missing option");

	if (! self->isset.LegacyTemperatureThresholdsBehaviour)
		self->LegacyTemperatureThresholdsBehaviour = false;

	if (false)
		return err_stringf("%s: %s", "LuaLibraries", "Missing option");

	if (! self->isset.EcPollInterval)
		self->EcPollInterval = 3000;

	if (! self->isset.CriticalTemperature)
		self->CriticalTemperature = 75;

	if (! self->isset.CriticalTemperatureOffset)
		self->CriticalTemperatureOffset = 15;

	if (! self->isset.ReadWriteWords)
		self->ReadWriteWords = false;

	if (false)
		return err_stringf("%s: %s", "Sponsor", "Missing option");

	if (false)
		return err_stringf("%s: %s", "FirmwareFingerprint", "Missing option");

	if (! self->isset.FanConfigurations)
		return err_stringf("%s: %s", "FanConfigurations", "Missing option");
	else if (! (self->FanConfigurations.size > 0))
		return err_stringf("%s: %s", "FanConfigurations", "requires: parameter.size > 0");

	if (false)
		return err_stringf("%s: %s", "RegisterWriteConfigurations", "Missing option");
	return err_success();
}

Error ModelConfig_FromJson(ModelConfig* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "NotebookModel")) {
			e = str_FromJson(&obj->NotebookModel, c);
			if (!e)
				obj->isset.NotebookModel = true;
		}
		else if (!strcmp(c->key, "Author")) {
			e = str_FromJson(&obj->Author, c);
			if (!e)
				obj->isset.Author = true;
		}
		else if (!strcmp(c->key, "LegacyTemperatureThresholdsBehaviour")) {
			e = bool_FromJson(&obj->LegacyTemperatureThresholdsBehaviour, c);
			if (!e)
				obj->isset.LegacyTemperatureThresholdsBehaviour = true;
		}
		else if (!strcmp(c->key, "LuaLibraries")) {
			e = LuaLibraries_FromJson(&obj->LuaLibraries, c);
			if (!e)
				obj->isset.LuaLibraries = true;
		}
		else if (!strcmp(c->key, "EcPollInterval")) {
			e = uint16_t_FromJson(&obj->EcPollInterval, c);
			if (!e)
				obj->isset.EcPollInterval = true;
		}
		else if (!strcmp(c->key, "CriticalTemperature")) {
			e = int16_t_FromJson(&obj->CriticalTemperature, c);
			if (!e)
				obj->isset.CriticalTemperature = true;
		}
		else if (!strcmp(c->key, "CriticalTemperatureOffset")) {
			e = uint16_t_FromJson(&obj->CriticalTemperatureOffset, c);
			if (!e)
				obj->isset.CriticalTemperatureOffset = true;
		}
		else if (!strcmp(c->key, "ReadWriteWords")) {
			e = bool_FromJson(&obj->ReadWriteWords, c);
			if (!e)
				obj->isset.ReadWriteWords = true;
		}
		else if (!strcmp(c->key, "Sponsor")) {
			e = Sponsor_FromJson(&obj->Sponsor, c);
			if (!e)
				obj->isset.Sponsor = true;
		}
		else if (!strcmp(c->key, "FirmwareFingerprint")) {
			e = array_of_str_FromJson(&obj->FirmwareFingerprint, c);
			if (!e)
				obj->isset.FirmwareFingerprint = true;
		}
		else if (!strcmp(c->key, "FanConfigurations")) {
			e = array_of_FanConfiguration_FromJson(&obj->FanConfigurations, c);
			if (!e)
				obj->isset.FanConfigurations = true;
		}
		else if (!strcmp(c->key, "RegisterWriteConfigurations")) {
			e = array_of_RegisterWriteConfiguration_FromJson(&obj->RegisterWriteConfigurations, c);
			if (!e)
				obj->isset.RegisterWriteConfigurations = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error FanTemperatureSourceConfig_ValidateFields(FanTemperatureSourceConfig* self) {
	(void) self;

	if (! self->isset.FanIndex)
		return err_stringf("%s: %s", "FanIndex", "Missing option");

	if (false)
		return err_stringf("%s: %s", "TemperatureAlgorithmType", "Missing option");

	if (false)
		return err_stringf("%s: %s", "Sensors", "Missing option");
	return err_success();
}

Error FanTemperatureSourceConfig_FromJson(FanTemperatureSourceConfig* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "FanIndex")) {
			e = uint8_t_FromJson(&obj->FanIndex, c);
			if (!e)
				obj->isset.FanIndex = true;
		}
		else if (!strcmp(c->key, "TemperatureAlgorithmType")) {
			e = TemperatureAlgorithmType_FromJson(&obj->TemperatureAlgorithmType, c);
			if (!e)
				obj->isset.TemperatureAlgorithmType = true;
		}
		else if (!strcmp(c->key, "Sensors")) {
			e = array_of_str_FromJson(&obj->Sensors, c);
			if (!e)
				obj->isset.Sensors = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error ServiceConfig_ValidateFields(ServiceConfig* self) {
	(void) self;

	if (! self->isset.SelectedConfigId)
		return err_stringf("%s: %s", "SelectedConfigId", "Missing option");

	if (false)
		return err_stringf("%s: %s", "EmbeddedControllerType", "Missing option");

	if (false)
		return err_stringf("%s: %s", "TargetFanSpeeds", "Missing option");

	if (false)
		return err_stringf("%s: %s", "FanTemperatureSources", "Missing option");
	return err_success();
}

Error ServiceConfig_FromJson(ServiceConfig* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "SelectedConfigId")) {
			e = str_FromJson(&obj->SelectedConfigId, c);
			if (!e)
				obj->isset.SelectedConfigId = true;
		}
		else if (!strcmp(c->key, "EmbeddedControllerType")) {
			e = EmbeddedControllerType_FromJson(&obj->EmbeddedControllerType, c);
			if (!e)
				obj->isset.EmbeddedControllerType = true;
		}
		else if (!strcmp(c->key, "TargetFanSpeeds")) {
			e = array_of_float_FromJson(&obj->TargetFanSpeeds, c);
			if (!e)
				obj->isset.TargetFanSpeeds = true;
		}
		else if (!strcmp(c->key, "FanTemperatureSources")) {
			e = array_of_FanTemperatureSourceConfig_FromJson(&obj->FanTemperatureSources, c);
			if (!e)
				obj->isset.FanTemperatureSources = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error ServiceState_ValidateFields(ServiceState* self) {
	(void) self;

	if (false)
		return err_stringf("%s: %s", "TargetFanSpeeds", "Missing option");
	return err_success();
}

Error ServiceState_FromJson(ServiceState* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "TargetFanSpeeds")) {
			e = array_of_float_FromJson(&obj->TargetFanSpeeds, c);
			if (!e)
				obj->isset.TargetFanSpeeds = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error FanInfo_ValidateFields(FanInfo* self) {
	(void) self;

	if (! self->isset.Name)
		return err_stringf("%s: %s", "Name", "Missing option");

	if (! self->isset.Temperature)
		return err_stringf("%s: %s", "Temperature", "Missing option");

	if (! self->isset.AutoMode)
		return err_stringf("%s: %s", "AutoMode", "Missing option");

	if (! self->isset.Critical)
		return err_stringf("%s: %s", "Critical", "Missing option");

	if (! self->isset.CurrentSpeed)
		return err_stringf("%s: %s", "CurrentSpeed", "Missing option");

	if (! self->isset.TargetSpeed)
		return err_stringf("%s: %s", "TargetSpeed", "Missing option");

	if (! self->isset.RequestedSpeed)
		return err_stringf("%s: %s", "RequestedSpeed", "Missing option");

	if (! self->isset.SpeedSteps)
		return err_stringf("%s: %s", "SpeedSteps", "Missing option");
	return err_success();
}

Error FanInfo_FromJson(FanInfo* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "Name")) {
			e = str_FromJson(&obj->Name, c);
			if (!e)
				obj->isset.Name = true;
		}
		else if (!strcmp(c->key, "Temperature")) {
			e = float_FromJson(&obj->Temperature, c);
			if (!e)
				obj->isset.Temperature = true;
		}
		else if (!strcmp(c->key, "AutoMode")) {
			e = bool_FromJson(&obj->AutoMode, c);
			if (!e)
				obj->isset.AutoMode = true;
		}
		else if (!strcmp(c->key, "Critical")) {
			e = bool_FromJson(&obj->Critical, c);
			if (!e)
				obj->isset.Critical = true;
		}
		else if (!strcmp(c->key, "CurrentSpeed")) {
			e = float_FromJson(&obj->CurrentSpeed, c);
			if (!e)
				obj->isset.CurrentSpeed = true;
		}
		else if (!strcmp(c->key, "TargetSpeed")) {
			e = float_FromJson(&obj->TargetSpeed, c);
			if (!e)
				obj->isset.TargetSpeed = true;
		}
		else if (!strcmp(c->key, "RequestedSpeed")) {
			e = float_FromJson(&obj->RequestedSpeed, c);
			if (!e)
				obj->isset.RequestedSpeed = true;
		}
		else if (!strcmp(c->key, "SpeedSteps")) {
			e = uint16_t_FromJson(&obj->SpeedSteps, c);
			if (!e)
				obj->isset.SpeedSteps = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

Error ServiceInfo_ValidateFields(ServiceInfo* self) {
	(void) self;

	if (! self->isset.PID)
		return err_stringf("%s: %s", "PID", "Missing option");

	if (! self->isset.SelectedConfigId)
		return err_stringf("%s: %s", "SelectedConfigId", "Missing option");

	if (! self->isset.ReadOnly)
		return err_stringf("%s: %s", "ReadOnly", "Missing option");

	if (! self->isset.Fans)
		return err_stringf("%s: %s", "Fans", "Missing option");
	return err_success();
}

Error ServiceInfo_FromJson(ServiceInfo* obj, const nx_json* json) {
	Error e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string("Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "#") || !strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "PID")) {
			e = int_FromJson(&obj->PID, c);
			if (!e)
				obj->isset.PID = true;
		}
		else if (!strcmp(c->key, "SelectedConfigId")) {
			e = str_FromJson(&obj->SelectedConfigId, c);
			if (!e)
				obj->isset.SelectedConfigId = true;
		}
		else if (!strcmp(c->key, "ReadOnly")) {
			e = bool_FromJson(&obj->ReadOnly, c);
			if (!e)
				obj->isset.ReadOnly = true;
		}
		else if (!strcmp(c->key, "Fans")) {
			e = array_of_FanInfo_FromJson(&obj->Fans, c);
			if (!e)
				obj->isset.Fans = true;
		}
		else
			e = err_string("Unknown option");
		if (e) return err_chain_string(e, c->key);
	}
	return err_success();
}

