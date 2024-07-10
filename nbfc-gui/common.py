def get_fan_temperature_sources_errors(fan_temperature_sources, fan_count, available_sensors):
    '''
    Checks `fan_temperature_sources` for errors.

    Args:
        fan_temperature_sources (list): A list of FanTemperatureSource objects.
        fan_count (int): The fan count of the service.
        available_sensors (list): A list of `SensorFile`.

    Returns:
        list: A list of error strings describing what's wrong. This list is
              empty if no errors are found.
    '''

    errors = []

    for i, fan_temperature_source in enumerate(fan_temperature_sources):
        fan_index = fan_temperature_source.get('FanIndex', None)

        # Chek for invalid FanIndex
        if fan_index is None:
            errors.append('FanTemperatureSources[%d]: Missing field: FanIndex' % i)

        elif fan_index < 0:
            errors.append('FanTemperatureSources[%d]: FanIndex: Cannot be negative' % i)

        elif fan_index >= fan_count:
            errors.append('FanTemperatureSources[%d]: FanIndex: No fan found for FanIndex `%d`' % (i, fan_index))

        # Check for invalid TemperatureAlgorithmType
        algorithm_type = fan_temperature_source.get('TemperatureAlgorithmType', 'Average')

        if algorithm_type not in ('Average', 'Max', 'Min'):
            errors.append('FanTemperatureSources[%d]: TemperatureAlgorithmType: Invalid value' % i)

        # Check for invalid Sensors
        sensors = fan_temperature_source.get('Sensors', [])

        for j, sensor in enumerate(sensors):
            found = False
            for available_sensor in available_sensors:
                if sensor == available_sensor.name or sensor == available_sensor.file:
                    found = True
                    break

            if not found:
                errors.append('FanTemperatureSources[%d]: Sensors[%d]: %s: Sensor not found' % (i, j, sensor))

        # Check for invalid fields
        for field in fan_temperature_source:
            if field not in ('FanIndex', 'TemperatureAlgorithmType', 'Sensors'):
                errors.append('FanTemperatureSources[%d]: Invalid field: %s' % (i, field))

    return errors

def fix_fan_temperature_sources(fan_temperature_sources, fan_count, available_sensors):
    '''
    Fixes a defect FanTemperatureSources config.

    Args:
        fan_temperature_sources (list): A list of FanTemperatureSource objects.
        fan_count (int): The fan count of the service.
        available_sensors (list): A list of `SensorFile`.

    Returns:
        list: A list of fixed FanTemperatureSource objects.
    '''

    result = []

    for fan_temperature_source in fan_temperature_sources:
        fan_index = fan_temperature_source.get('FanIndex', None)

        # Drop FanTemperatureSource if FanIndex is not valid
        if fan_index is None:      continue
        if fan_index < 0:          continue
        if fan_index >= fan_count: continue

        # Set TemperatureAlgorithmType to 'Average' if invalid
        algorithm_type = fan_temperature_source.get('TemperatureAlgorithmType', 'Average')

        if algorithm_type not in ('Average', 'Max', 'Min'):
            algorithm_type = 'Average'

        # Drop invalid sensors
        sensors = []

        for sensor in fan_temperature_source.get('Sensors', []):
            found = False
            for available_sensor in available_sensors:
                if sensor == available_sensor.name or sensor == available_sensor.file:
                    found = True
                    break

            if found:
                sensors.append(sensor)

        obj = {'FanIndex': fan_index}
        if sensors:
            obj['Sensors'] = sensors
        if algorithm_type != 'Average':
            obj['TemperatureAlgorithmType'] = algorithm_type

        result.append(obj)

    return result

