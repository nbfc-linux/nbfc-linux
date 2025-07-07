/* Auto generated code ['./tools/config.py', 'source'] */

Error* TemperatureThreshold_ValidateFields(TemperatureThreshold* self) {
	if (! TemperatureThreshold_IsSet_UpThreshold(self))
		return err_stringf(0, "%s: %s", "UpThreshold", "Missing option");

	if (! TemperatureThreshold_IsSet_DownThreshold(self))
		return err_stringf(0, "%s: %s", "DownThreshold", "Missing option");

	if (! TemperatureThreshold_IsSet_FanSpeed(self))
		return err_stringf(0, "%s: %s", "FanSpeed", "Missing option");
	else if (! (self->FanSpeed >= 0.0 && self->FanSpeed <= 100.0))
		return err_stringf(0, "%s: %s", "FanSpeed", "requires: parameter >= 0.0 && parameter <= 100.0");
	return err_success();
}

Error* TemperatureThreshold_FromJson(TemperatureThreshold* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "UpThreshold")) {
			e = int16_t_FromJson(&obj->UpThreshold, c);
			if (!e)
				TemperatureThreshold_Set_UpThreshold(obj);
		}
		else if (!strcmp(c->key, "DownThreshold")) {
			e = int16_t_FromJson(&obj->DownThreshold, c);
			if (!e)
				TemperatureThreshold_Set_DownThreshold(obj);
		}
		else if (!strcmp(c->key, "FanSpeed")) {
			e = float_FromJson(&obj->FanSpeed, c);
			if (!e)
				TemperatureThreshold_Set_FanSpeed(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* FanSpeedPercentageOverride_ValidateFields(FanSpeedPercentageOverride* self) {
	if (! FanSpeedPercentageOverride_IsSet_FanSpeedPercentage(self))
		return err_stringf(0, "%s: %s", "FanSpeedPercentage", "Missing option");
	else if (! (self->FanSpeedPercentage >= 0.0 && self->FanSpeedPercentage <= 100.0))
		return err_stringf(0, "%s: %s", "FanSpeedPercentage", "requires: parameter >= 0.0 && parameter <= 100.0");

	if (! FanSpeedPercentageOverride_IsSet_FanSpeedValue(self))
		return err_stringf(0, "%s: %s", "FanSpeedValue", "Missing option");

	if (! FanSpeedPercentageOverride_IsSet_TargetOperation(self))
		self->TargetOperation = OverrideTargetOperation_ReadWrite;
	return err_success();
}

Error* FanSpeedPercentageOverride_FromJson(FanSpeedPercentageOverride* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "FanSpeedPercentage")) {
			e = float_FromJson(&obj->FanSpeedPercentage, c);
			if (!e)
				FanSpeedPercentageOverride_Set_FanSpeedPercentage(obj);
		}
		else if (!strcmp(c->key, "FanSpeedValue")) {
			e = uint16_t_FromJson(&obj->FanSpeedValue, c);
			if (!e)
				FanSpeedPercentageOverride_Set_FanSpeedValue(obj);
		}
		else if (!strcmp(c->key, "TargetOperation")) {
			e = OverrideTargetOperation_FromJson(&obj->TargetOperation, c);
			if (!e)
				FanSpeedPercentageOverride_Set_TargetOperation(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* RegisterWriteConfiguration_ValidateFields(RegisterWriteConfiguration* self) {
	if (! RegisterWriteConfiguration_IsSet_WriteMode(self))
		self->WriteMode = RegisterWriteMode_Set;

	if (! RegisterWriteConfiguration_IsSet_WriteOccasion(self))
		self->WriteOccasion = RegisterWriteOccasion_OnInitialization;

	if (false)
		return err_stringf(0, "%s: %s", "Register", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "Value", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "AcpiMethod", "Missing option");

	if (! RegisterWriteConfiguration_IsSet_ResetRequired(self))
		self->ResetRequired = false;

	if (false)
		return err_stringf(0, "%s: %s", "ResetValue", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "ResetAcpiMethod", "Missing option");

	if (! RegisterWriteConfiguration_IsSet_ResetWriteMode(self))
		self->ResetWriteMode = RegisterWriteMode_Set;

	if (! RegisterWriteConfiguration_IsSet_Description(self))
		self->Description = Mem_Strdup("");
	return err_success();
}

Error* RegisterWriteConfiguration_FromJson(RegisterWriteConfiguration* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "WriteMode")) {
			e = RegisterWriteMode_FromJson(&obj->WriteMode, c);
			if (!e)
				RegisterWriteConfiguration_Set_WriteMode(obj);
		}
		else if (!strcmp(c->key, "WriteOccasion")) {
			e = RegisterWriteOccasion_FromJson(&obj->WriteOccasion, c);
			if (!e)
				RegisterWriteConfiguration_Set_WriteOccasion(obj);
		}
		else if (!strcmp(c->key, "Register")) {
			e = uint8_t_FromJson(&obj->Register, c);
			if (!e)
				RegisterWriteConfiguration_Set_Register(obj);
		}
		else if (!strcmp(c->key, "Value")) {
			e = uint16_t_FromJson(&obj->Value, c);
			if (!e)
				RegisterWriteConfiguration_Set_Value(obj);
		}
		else if (!strcmp(c->key, "AcpiMethod")) {
			e = str_FromJson(&obj->AcpiMethod, c);
			if (!e)
				RegisterWriteConfiguration_Set_AcpiMethod(obj);
		}
		else if (!strcmp(c->key, "ResetRequired")) {
			e = bool_FromJson(&obj->ResetRequired, c);
			if (!e)
				RegisterWriteConfiguration_Set_ResetRequired(obj);
		}
		else if (!strcmp(c->key, "ResetValue")) {
			e = uint16_t_FromJson(&obj->ResetValue, c);
			if (!e)
				RegisterWriteConfiguration_Set_ResetValue(obj);
		}
		else if (!strcmp(c->key, "ResetAcpiMethod")) {
			e = str_FromJson(&obj->ResetAcpiMethod, c);
			if (!e)
				RegisterWriteConfiguration_Set_ResetAcpiMethod(obj);
		}
		else if (!strcmp(c->key, "ResetWriteMode")) {
			e = RegisterWriteMode_FromJson(&obj->ResetWriteMode, c);
			if (!e)
				RegisterWriteConfiguration_Set_ResetWriteMode(obj);
		}
		else if (!strcmp(c->key, "Description")) {
			e = str_FromJson(&obj->Description, c);
			if (!e)
				RegisterWriteConfiguration_Set_Description(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* FanConfiguration_ValidateFields(FanConfiguration* self) {
	if (false)
		return err_stringf(0, "%s: %s", "FanDisplayName", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "ReadRegister", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "ReadAcpiMethod", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "WriteRegister", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "WriteAcpiMethod", "Missing option");

	if (! FanConfiguration_IsSet_MinSpeedValue(self))
		return err_stringf(0, "%s: %s", "MinSpeedValue", "Missing option");

	if (! FanConfiguration_IsSet_MaxSpeedValue(self))
		return err_stringf(0, "%s: %s", "MaxSpeedValue", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "MinSpeedValueRead", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "MaxSpeedValueRead", "Missing option");

	if (! FanConfiguration_IsSet_IndependentReadMinMaxValues(self))
		self->IndependentReadMinMaxValues = false;

	if (! FanConfiguration_IsSet_ResetRequired(self))
		self->ResetRequired = false;

	if (false)
		return err_stringf(0, "%s: %s", "FanSpeedResetValue", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "ResetAcpiMethod", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "TemperatureAlgorithmType", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "Sensors", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "TemperatureThresholds", "Missing option");

	if (! FanConfiguration_IsSet_FanSpeedPercentageOverrides(self))
		self->FanSpeedPercentageOverrides = Config_DefaultFanSpeedPercentageOverrides;
	return err_success();
}

Error* FanConfiguration_FromJson(FanConfiguration* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "FanDisplayName")) {
			e = str_FromJson(&obj->FanDisplayName, c);
			if (!e)
				FanConfiguration_Set_FanDisplayName(obj);
		}
		else if (!strcmp(c->key, "ReadRegister")) {
			e = uint8_t_FromJson(&obj->ReadRegister, c);
			if (!e)
				FanConfiguration_Set_ReadRegister(obj);
		}
		else if (!strcmp(c->key, "ReadAcpiMethod")) {
			e = str_FromJson(&obj->ReadAcpiMethod, c);
			if (!e)
				FanConfiguration_Set_ReadAcpiMethod(obj);
		}
		else if (!strcmp(c->key, "WriteRegister")) {
			e = uint8_t_FromJson(&obj->WriteRegister, c);
			if (!e)
				FanConfiguration_Set_WriteRegister(obj);
		}
		else if (!strcmp(c->key, "WriteAcpiMethod")) {
			e = str_FromJson(&obj->WriteAcpiMethod, c);
			if (!e)
				FanConfiguration_Set_WriteAcpiMethod(obj);
		}
		else if (!strcmp(c->key, "MinSpeedValue")) {
			e = uint16_t_FromJson(&obj->MinSpeedValue, c);
			if (!e)
				FanConfiguration_Set_MinSpeedValue(obj);
		}
		else if (!strcmp(c->key, "MaxSpeedValue")) {
			e = uint16_t_FromJson(&obj->MaxSpeedValue, c);
			if (!e)
				FanConfiguration_Set_MaxSpeedValue(obj);
		}
		else if (!strcmp(c->key, "MinSpeedValueRead")) {
			e = uint16_t_FromJson(&obj->MinSpeedValueRead, c);
			if (!e)
				FanConfiguration_Set_MinSpeedValueRead(obj);
		}
		else if (!strcmp(c->key, "MaxSpeedValueRead")) {
			e = uint16_t_FromJson(&obj->MaxSpeedValueRead, c);
			if (!e)
				FanConfiguration_Set_MaxSpeedValueRead(obj);
		}
		else if (!strcmp(c->key, "IndependentReadMinMaxValues")) {
			e = bool_FromJson(&obj->IndependentReadMinMaxValues, c);
			if (!e)
				FanConfiguration_Set_IndependentReadMinMaxValues(obj);
		}
		else if (!strcmp(c->key, "ResetRequired")) {
			e = bool_FromJson(&obj->ResetRequired, c);
			if (!e)
				FanConfiguration_Set_ResetRequired(obj);
		}
		else if (!strcmp(c->key, "FanSpeedResetValue")) {
			e = uint16_t_FromJson(&obj->FanSpeedResetValue, c);
			if (!e)
				FanConfiguration_Set_FanSpeedResetValue(obj);
		}
		else if (!strcmp(c->key, "ResetAcpiMethod")) {
			e = str_FromJson(&obj->ResetAcpiMethod, c);
			if (!e)
				FanConfiguration_Set_ResetAcpiMethod(obj);
		}
		else if (!strcmp(c->key, "TemperatureAlgorithmType")) {
			e = TemperatureAlgorithmType_FromJson(&obj->TemperatureAlgorithmType, c);
			if (!e)
				FanConfiguration_Set_TemperatureAlgorithmType(obj);
		}
		else if (!strcmp(c->key, "Sensors")) {
			e = array_of_str_FromJson(&obj->Sensors, c);
			if (!e)
				FanConfiguration_Set_Sensors(obj);
		}
		else if (!strcmp(c->key, "TemperatureThresholds")) {
			e = array_of_TemperatureThreshold_FromJson(&obj->TemperatureThresholds, c);
			if (!e)
				FanConfiguration_Set_TemperatureThresholds(obj);
		}
		else if (!strcmp(c->key, "FanSpeedPercentageOverrides")) {
			e = array_of_FanSpeedPercentageOverride_FromJson(&obj->FanSpeedPercentageOverrides, c);
			if (!e)
				FanConfiguration_Set_FanSpeedPercentageOverrides(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* Sponsor_ValidateFields(Sponsor* self) {
	if (! Sponsor_IsSet_Name(self))
		return err_stringf(0, "%s: %s", "Name", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "Description", "Missing option");

	if (! Sponsor_IsSet_URL(self))
		return err_stringf(0, "%s: %s", "URL", "Missing option");

	if (! Sponsor_IsSet_BannerURL(self))
		return err_stringf(0, "%s: %s", "BannerURL", "Missing option");
	return err_success();
}

Error* Sponsor_FromJson(Sponsor* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "Name")) {
			e = str_FromJson(&obj->Name, c);
			if (!e)
				Sponsor_Set_Name(obj);
		}
		else if (!strcmp(c->key, "Description")) {
			e = str_FromJson(&obj->Description, c);
			if (!e)
				Sponsor_Set_Description(obj);
		}
		else if (!strcmp(c->key, "URL")) {
			e = str_FromJson(&obj->URL, c);
			if (!e)
				Sponsor_Set_URL(obj);
		}
		else if (!strcmp(c->key, "BannerURL")) {
			e = str_FromJson(&obj->BannerURL, c);
			if (!e)
				Sponsor_Set_BannerURL(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* ModelConfig_ValidateFields(ModelConfig* self) {
	if (! ModelConfig_IsSet_NotebookModel(self))
		return err_stringf(0, "%s: %s", "NotebookModel", "Missing option");

	if (! ModelConfig_IsSet_Author(self))
		self->Author = Mem_Strdup("");

	if (! ModelConfig_IsSet_LegacyTemperatureThresholdsBehaviour(self))
		self->LegacyTemperatureThresholdsBehaviour = false;

	if (! ModelConfig_IsSet_EcPollInterval(self))
		self->EcPollInterval = 3000;

	if (! ModelConfig_IsSet_CriticalTemperature(self))
		self->CriticalTemperature = 75;

	if (! ModelConfig_IsSet_CriticalTemperatureOffset(self))
		self->CriticalTemperatureOffset = 15;

	if (! ModelConfig_IsSet_ReadWriteWords(self))
		self->ReadWriteWords = false;

	if (false)
		return err_stringf(0, "%s: %s", "Sponsor", "Missing option");

	if (! ModelConfig_IsSet_FanConfigurations(self))
		return err_stringf(0, "%s: %s", "FanConfigurations", "Missing option");
	else if (! (self->FanConfigurations.size > 0))
		return err_stringf(0, "%s: %s", "FanConfigurations", "requires: parameter.size > 0");

	if (false)
		return err_stringf(0, "%s: %s", "RegisterWriteConfigurations", "Missing option");
	return err_success();
}

Error* ModelConfig_FromJson(ModelConfig* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "NotebookModel")) {
			e = str_FromJson(&obj->NotebookModel, c);
			if (!e)
				ModelConfig_Set_NotebookModel(obj);
		}
		else if (!strcmp(c->key, "Author")) {
			e = str_FromJson(&obj->Author, c);
			if (!e)
				ModelConfig_Set_Author(obj);
		}
		else if (!strcmp(c->key, "LegacyTemperatureThresholdsBehaviour")) {
			e = bool_FromJson(&obj->LegacyTemperatureThresholdsBehaviour, c);
			if (!e)
				ModelConfig_Set_LegacyTemperatureThresholdsBehaviour(obj);
		}
		else if (!strcmp(c->key, "EcPollInterval")) {
			e = uint16_t_FromJson(&obj->EcPollInterval, c);
			if (!e)
				ModelConfig_Set_EcPollInterval(obj);
		}
		else if (!strcmp(c->key, "CriticalTemperature")) {
			e = int16_t_FromJson(&obj->CriticalTemperature, c);
			if (!e)
				ModelConfig_Set_CriticalTemperature(obj);
		}
		else if (!strcmp(c->key, "CriticalTemperatureOffset")) {
			e = uint16_t_FromJson(&obj->CriticalTemperatureOffset, c);
			if (!e)
				ModelConfig_Set_CriticalTemperatureOffset(obj);
		}
		else if (!strcmp(c->key, "ReadWriteWords")) {
			e = bool_FromJson(&obj->ReadWriteWords, c);
			if (!e)
				ModelConfig_Set_ReadWriteWords(obj);
		}
		else if (!strcmp(c->key, "Sponsor")) {
			e = Sponsor_FromJson(&obj->Sponsor, c);
			if (!e)
				ModelConfig_Set_Sponsor(obj);
		}
		else if (!strcmp(c->key, "FanConfigurations")) {
			e = array_of_FanConfiguration_FromJson(&obj->FanConfigurations, c);
			if (!e)
				ModelConfig_Set_FanConfigurations(obj);
		}
		else if (!strcmp(c->key, "RegisterWriteConfigurations")) {
			e = array_of_RegisterWriteConfiguration_FromJson(&obj->RegisterWriteConfigurations, c);
			if (!e)
				ModelConfig_Set_RegisterWriteConfigurations(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* FanTemperatureSourceConfig_ValidateFields(FanTemperatureSourceConfig* self) {
	if (! FanTemperatureSourceConfig_IsSet_FanIndex(self))
		return err_stringf(0, "%s: %s", "FanIndex", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "TemperatureAlgorithmType", "Missing option");

	if (false)
		return err_stringf(0, "%s: %s", "Sensors", "Missing option");
	return err_success();
}

Error* FanTemperatureSourceConfig_FromJson(FanTemperatureSourceConfig* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "FanIndex")) {
			e = uint8_t_FromJson(&obj->FanIndex, c);
			if (!e)
				FanTemperatureSourceConfig_Set_FanIndex(obj);
		}
		else if (!strcmp(c->key, "TemperatureAlgorithmType")) {
			e = TemperatureAlgorithmType_FromJson(&obj->TemperatureAlgorithmType, c);
			if (!e)
				FanTemperatureSourceConfig_Set_TemperatureAlgorithmType(obj);
		}
		else if (!strcmp(c->key, "Sensors")) {
			e = array_of_str_FromJson(&obj->Sensors, c);
			if (!e)
				FanTemperatureSourceConfig_Set_Sensors(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* ServiceConfig_ValidateFields(ServiceConfig* self) {
	if (! ServiceConfig_IsSet_SelectedConfigId(self))
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
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "SelectedConfigId")) {
			e = str_FromJson(&obj->SelectedConfigId, c);
			if (!e)
				ServiceConfig_Set_SelectedConfigId(obj);
		}
		else if (!strcmp(c->key, "EmbeddedControllerType")) {
			e = EmbeddedControllerType_FromJson(&obj->EmbeddedControllerType, c);
			if (!e)
				ServiceConfig_Set_EmbeddedControllerType(obj);
		}
		else if (!strcmp(c->key, "TargetFanSpeeds")) {
			e = array_of_float_FromJson(&obj->TargetFanSpeeds, c);
			if (!e)
				ServiceConfig_Set_TargetFanSpeeds(obj);
		}
		else if (!strcmp(c->key, "FanTemperatureSources")) {
			e = array_of_FanTemperatureSourceConfig_FromJson(&obj->FanTemperatureSources, c);
			if (!e)
				ServiceConfig_Set_FanTemperatureSources(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* ServiceState_ValidateFields(ServiceState* self) {
	if (false)
		return err_stringf(0, "%s: %s", "TargetFanSpeeds", "Missing option");
	return err_success();
}

Error* ServiceState_FromJson(ServiceState* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "TargetFanSpeeds")) {
			e = array_of_float_FromJson(&obj->TargetFanSpeeds, c);
			if (!e)
				ServiceState_Set_TargetFanSpeeds(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* FanInfo_ValidateFields(FanInfo* self) {
	if (! FanInfo_IsSet_Name(self))
		return err_stringf(0, "%s: %s", "Name", "Missing option");

	if (! FanInfo_IsSet_Temperature(self))
		return err_stringf(0, "%s: %s", "Temperature", "Missing option");

	if (! FanInfo_IsSet_AutoMode(self))
		return err_stringf(0, "%s: %s", "AutoMode", "Missing option");

	if (! FanInfo_IsSet_Critical(self))
		return err_stringf(0, "%s: %s", "Critical", "Missing option");

	if (! FanInfo_IsSet_CurrentSpeed(self))
		return err_stringf(0, "%s: %s", "CurrentSpeed", "Missing option");

	if (! FanInfo_IsSet_TargetSpeed(self))
		return err_stringf(0, "%s: %s", "TargetSpeed", "Missing option");

	if (! FanInfo_IsSet_RequestedSpeed(self))
		return err_stringf(0, "%s: %s", "RequestedSpeed", "Missing option");

	if (! FanInfo_IsSet_SpeedSteps(self))
		return err_stringf(0, "%s: %s", "SpeedSteps", "Missing option");
	return err_success();
}

Error* FanInfo_FromJson(FanInfo* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "Name")) {
			e = str_FromJson(&obj->Name, c);
			if (!e)
				FanInfo_Set_Name(obj);
		}
		else if (!strcmp(c->key, "Temperature")) {
			e = float_FromJson(&obj->Temperature, c);
			if (!e)
				FanInfo_Set_Temperature(obj);
		}
		else if (!strcmp(c->key, "AutoMode")) {
			e = bool_FromJson(&obj->AutoMode, c);
			if (!e)
				FanInfo_Set_AutoMode(obj);
		}
		else if (!strcmp(c->key, "Critical")) {
			e = bool_FromJson(&obj->Critical, c);
			if (!e)
				FanInfo_Set_Critical(obj);
		}
		else if (!strcmp(c->key, "CurrentSpeed")) {
			e = float_FromJson(&obj->CurrentSpeed, c);
			if (!e)
				FanInfo_Set_CurrentSpeed(obj);
		}
		else if (!strcmp(c->key, "TargetSpeed")) {
			e = float_FromJson(&obj->TargetSpeed, c);
			if (!e)
				FanInfo_Set_TargetSpeed(obj);
		}
		else if (!strcmp(c->key, "RequestedSpeed")) {
			e = float_FromJson(&obj->RequestedSpeed, c);
			if (!e)
				FanInfo_Set_RequestedSpeed(obj);
		}
		else if (!strcmp(c->key, "SpeedSteps")) {
			e = uint16_t_FromJson(&obj->SpeedSteps, c);
			if (!e)
				FanInfo_Set_SpeedSteps(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

Error* ServiceInfo_ValidateFields(ServiceInfo* self) {
	if (! ServiceInfo_IsSet_PID(self))
		return err_stringf(0, "%s: %s", "PID", "Missing option");

	if (! ServiceInfo_IsSet_SelectedConfigId(self))
		return err_stringf(0, "%s: %s", "SelectedConfigId", "Missing option");

	if (! ServiceInfo_IsSet_ReadOnly(self))
		return err_stringf(0, "%s: %s", "ReadOnly", "Missing option");

	if (! ServiceInfo_IsSet_Fans(self))
		return err_stringf(0, "%s: %s", "Fans", "Missing option");
	return err_success();
}

Error* ServiceInfo_FromJson(ServiceInfo* obj, const nx_json* json) {
	Error* e;
	memset(obj, 0, sizeof(*obj));

	if (!json || json->type != NX_JSON_OBJECT)
		return err_string(0, "Not a JSON object");

	nx_json_for_each(c, json) {
		if (!strcmp(c->key, "Comment"))
			continue;
		else if (!strcmp(c->key, "PID")) {
			e = int_FromJson(&obj->PID, c);
			if (!e)
				ServiceInfo_Set_PID(obj);
		}
		else if (!strcmp(c->key, "SelectedConfigId")) {
			e = str_FromJson(&obj->SelectedConfigId, c);
			if (!e)
				ServiceInfo_Set_SelectedConfigId(obj);
		}
		else if (!strcmp(c->key, "ReadOnly")) {
			e = bool_FromJson(&obj->ReadOnly, c);
			if (!e)
				ServiceInfo_Set_ReadOnly(obj);
		}
		else if (!strcmp(c->key, "Fans")) {
			e = array_of_FanInfo_FromJson(&obj->Fans, c);
			if (!e)
				ServiceInfo_Set_Fans(obj);
		}
		else
			e = err_string(0, "Unknown option");
		if (e) return err_string(e, c->key);
	}
	return err_success();
}

