#!/usr/bin/env python3

import os
import sys
import json
import signal

from PyQt5.QtWidgets import *
from PyQt5.QtCore import Qt, QTimer, QObject, pyqtSignal

ABOUT_NBFC_LINUX_QT = """\
NBFC-Linux is a fan control utility desined for Linux systems. <br />
<br />
<b>Author</b>: <a href="https://github.com/braph">Benjamin Abendroth</a> <br />
<br />
<b>License</b>: GPL-3.0 <br />
<br />
<b>Project Homepage</b>: <a href="https://github.com/nbfc-linux/nbfc-linux">GitHub.com/nbfc-linux/nbfc-linux</a> <br />
<br />
<b>AUR Packages</b>: Provided by <a href="https://github.com/BachoSeven">Francesco Minnocci</a>
"""
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

#!/usr/bin/python3

import os
from collections import namedtuple

LinuxHwmonDirs = [
  "/sys/class/hwmon/hwmon%d",
  "/sys/class/hwmon/hwmon%d/device",
]

LinuxTempSensorFile = "temp%d_input"

SensorFile = namedtuple('SensorFile', ['name', 'file'])

def get_sensors():
    result = []

    for hwmonDir in LinuxHwmonDirs:
        for i in range(10):
            dir = hwmonDir % i
            file = "%s/name" % dir

            try:
                with open(file, 'r') as fh:
                    source_name = fh.read().strip()
            except:
                continue

            for j in range(10):
                filename = LinuxTempSensorFile % j
                filename = "%s/%s" % (dir, filename)

                if os.path.exists(filename):
                    result.append(SensorFile(source_name, filename))

    return result
#!/usr/bin/env python3

import os
import json
import socket
import subprocess

class NbfcClient:
    '''
    A client to interact with the NBFC service using Unix sockets.
    '''

    def __init__(self):
        '''
        Initializes the NbfcClient instance by retrieving necessary file paths.
        '''

        self.socket_file = self.call_nbfc(['show-variable', 'socket_file'])
        self.config_file = self.call_nbfc(['show-variable', 'config_file'])
        self.model_configs_dir = self.call_nbfc(['show-variable', 'model_configs_dir'])

    def call_nbfc(self, args):
        '''
        Calls the NBFC client binary with the given arguments and returns the output.

        Args:
            args(list): The arguments to pass to the NBFC client.

        Returns:
            str: The output from the NBFC client command.

        Raises:
            Exception: If the NBFC client command returns a non-zero exit code.
                       The exception's text is the output written to STDERR.
        '''

        command = ['nbfc', '--python-hack'] + args
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode != 0:
            raise Exception(result.stderr.rstrip())
        return result.stdout.rstrip()

    def socket_communicate(self, data):
        '''
        Sends a JSON-encoded message to the NBFC service via a Unix socket
        and returns the response.

        Args:
            data(dict): The data to send to the NBFC service.

        Returns:
            dict: The response from the NBFC service.

        Raises:
            Exception: If there is an error in the communication with the socket.
        '''

        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
            sock.connect(self.socket_file)

            message = "%s\nEND" % json.dumps(data)
            sock.sendall(message.encode('utf-8'))

            response = b''
            while True:
                data = sock.recv(1024)
                response += data
                if b'\nEND' in response:
                    break
            
            response = response.decode('utf-8')
            response = response.replace('\nEND', '')
            response = json.loads(response)
            return response

    def start(self, readonly=False):
        '''
        Starts the NBFC service.

        Args:
            readonly (bool): If True, starts the service in read-only mode.
        '''

        args = ['start']
        if readonly:
            args.append('-r')

        self.call_nbfc(args)

    def restart(self, readonly=False):
        '''
        Restarts the NBFC service.

        Args:
            readonly (bool): If True, starts the service in read-only mode.
        '''

        args = ['restart']
        if readonly:
            args.append('-r')

        self.call_nbfc(args)

    def stop(self):
        '''
        Stops the NBFC service.
        '''

        self.call_nbfc(['stop'])

    def get_status(self):
        '''
        Retrieves the status of the NBFC service.

        Returns:
            dict: The status information of the NBFC service.

        Raises:
            Exception: If there is an error in the response from the service.
        '''

        response = self.socket_communicate({'Command': 'status'})
        if 'Error' in response:
            raise Exception(response['Error'])
        return response

    def set_fan_speed(self, speed, fan=None):
        '''
        Sets the fan speed.

        Args:
            speed (float, int, str): The desired fan speed in percent or 'auto'
                                     for setting fan to auto-mode.

            fan (int, optional): The fan index to set the speed for. If not given,
                                 set the speed for all available fans.

        Raises:
            Exception: If there is an error in the response from the service.
        '''

        request = {'Command': 'set-fan-speed', 'Speed': speed}
        if fan is not None:
            request['Fan'] = fan
        response = self.socket_communicate(request)
        if 'Error' in response:
            raise Exception(response['Error'])

    def get_model_name(self):
        '''
        Retrieve the model name of the notebook.

        Returns:
            str: The model name of the notebook.
        '''

        return self.call_nbfc(['get-model-name'])

    def list_configs(self):
        '''
        List all available model configurations.

        Returns:
            list: A list of all available model configurations.
        '''

        configs = self.call_nbfc(['config', '-l'])
        if configs:
            return configs.split('\n')
        else:
            return []

    def recommended_configs(self):
        '''
        List recommended model configurations.

        Returns:
            list: A list of recommended configurations.

        Note:
            This returns recommended configurations based solely on comparing
            your model name with configuration file names.
            This recommendation does not imply any further significance or validation
            of the configurations beyond the string matching.
        '''

        configs = self.call_nbfc(['config', '-r'])
        if configs:
            return configs.split('\n')
        else:
            return []

    def get_config(self):
        '''
        Retrieves the current configuration.

        Returns:
            dict: The current configuration.

        Raises:
            JSONDecodeError: If the configuration file is not valid JSON.
        '''

        try:
            with open(self.config_file, 'r') as fh:
                return json.load(fh)
        except FileNotFoundError:
            return {}

    def set_config(self, config):
        '''
        Writes a new configuration to the config file.

        Args:
            config (dict): The configuration data to write.

        Raises:
            PermissionError: If the program does not have permission to write
                             to the config file.
        '''

        with open(self.config_file, 'w') as fh:
            return json.dump(config, fh, indent=1)


IS_ROOT = (os.geteuid() == 0)
NBFC_CLIENT = NbfcClient()
NOT_ROOT_NOTICE = "You cannot change the configuration because you are not root"

class ModelConfigChanged(QObject):
    changed = pyqtSignal()

MODEL_CONFIG_CHANGED = ModelConfigChanged()

class ApplyButtonsWidget(QWidget):
    def __init__(self):
        super().__init__()

        vbox_layout = QVBoxLayout()
        #vbox_layout.addStretch() TODO
        self.setLayout(vbox_layout)

        self.error_label = QLabel("", self)
        vbox_layout.addWidget(self.error_label)

        self.read_only_checkbox = QCheckBox("(Re-)start in read-only mode", self)
        vbox_layout.addWidget(self.read_only_checkbox)

        hbox_layout = QHBoxLayout()
        vbox_layout.addLayout(hbox_layout)

        self.apply_button = QPushButton("Apply", self)
        hbox_layout.addWidget(self.apply_button)

        self.apply_with_restart_button = QPushButton("Apply with (re-)start", self)
        hbox_layout.addWidget(self.apply_with_restart_button)

    def enable(self):
        self.error_label.setHidden(True)
        self.read_only_checkbox.setEnabled(True)
        self.apply_button.setEnabled(True)
        self.apply_with_restart_button.setEnabled(True)

    def disable(self, reason):
        self.error_label.setText(reason)
        self.error_label.setHidden(False)
        self.read_only_checkbox.setEnabled(False)
        self.apply_button.setEnabled(False)
        self.apply_with_restart_button.setEnabled(False)

class FanWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.fan_index = None

        layout = QVBoxLayout()
        self.setLayout(layout)

        grid_layout = QGridLayout()
        layout.addLayout(grid_layout)

        label = QLabel("Name", self)
        self.name_label = QLabel("", self)
        grid_layout.addWidget(label, 0, 0)
        grid_layout.addWidget(self.name_label, 0, 1)

        label = QLabel("Temperature", self)
        self.temperature_label = QLabel("", self)
        grid_layout.addWidget(label, 1, 0)
        grid_layout.addWidget(self.temperature_label, 1, 1)

        label = QLabel("Auto mode", self)
        self.auto_mode_label = QLabel("", self)
        grid_layout.addWidget(label, 2, 0)
        grid_layout.addWidget(self.auto_mode_label, 2, 1)

        label = QLabel("Critical", self)
        self.critical_label = QLabel("", self)
        grid_layout.addWidget(label, 3, 0)
        grid_layout.addWidget(self.critical_label, 3, 1)

        label = QLabel("Current speed", self)
        self.current_speed_label = QLabel("", self)
        grid_layout.addWidget(label, 4, 0)
        grid_layout.addWidget(self.current_speed_label, 4, 1)

        label = QLabel("Target speed", self)
        self.target_speed_label = QLabel("", self)
        grid_layout.addWidget(label, 5, 0)
        grid_layout.addWidget(self.target_speed_label, 5, 1)

        label = QLabel("Speed steps", self)
        self.speed_steps_label = QLabel("", self)
        grid_layout.addWidget(label, 6, 0)
        grid_layout.addWidget(self.speed_steps_label, 6, 1)

        self.auto_mode_checkbox = QCheckBox("Auto mode", self)
        self.auto_mode_checkbox.stateChanged.connect(self.update_fan_speed)
        layout.addWidget(self.auto_mode_checkbox)

        self.speed_slider = QSlider(Qt.Horizontal)
        self.speed_slider.setMinimum(0)
        self.speed_slider.setMaximum(100)
        self.speed_slider.setTickInterval(1)
        self.speed_slider.valueChanged.connect(self.update_fan_speed)
        layout.addWidget(self.speed_slider)

    def update_fan_speed(self, *_):
        if self.auto_mode_checkbox.isChecked():
            NBFC_CLIENT.set_fan_speed('auto', self.fan_index)
        else:
            NBFC_CLIENT.set_fan_speed(self.speed_slider.value(), self.fan_index)

    def update(self, fan_index, fan_data):
        self.fan_index = fan_index
        self.name_label.setText(fan_data['Name'])
        self.temperature_label.setText(str(fan_data['Temperature']))
        self.auto_mode_label.setText(str(fan_data['AutoMode']))
        self.critical_label.setText(str(fan_data['Critical']))
        self.current_speed_label.setText(str(fan_data['CurrentSpeed']))
        self.target_speed_label.setText(str(fan_data['TargetSpeed']))
        self.speed_steps_label.setText(str(fan_data['SpeedSteps']))
        self.auto_mode_checkbox.setChecked(fan_data['AutoMode'])
        self.speed_slider.setValue(int(fan_data['RequestedSpeed']))

class FanControlWidget(QStackedWidget):
    def __init__(self):
        super().__init__()

        self.timer = QTimer(self)
        self.timer.setInterval(500)
        self.timer.timeout.connect(self.update)

        self.error_widget = QWidget()
        error_layout = QVBoxLayout()
        self.error_widget.setLayout(error_layout)
        self.error_label = QLabel("", self)
        error_layout.addWidget(self.error_label)
        self.addWidget(self.error_widget)

        self.scroll_area = QScrollArea()
        self.scroll_area.setWidgetResizable(True)
        fans_widget = QWidget()
        self.fans_layout = QVBoxLayout()
        fans_widget.setLayout(self.fans_layout)
        self.scroll_area.setWidget(fans_widget)
        self.addWidget(self.scroll_area)

    def start(self):
        self.update()
        self.timer.start()

    def stop(self):
        self.timer.stop()

    def update(self):
        try:
            status = NBFC_CLIENT.get_status()
            self.setCurrentWidget(self.scroll_area)
        except Exception as e:
            self.error_label.setText(str(e))
            self.setCurrentWidget(self.error_widget)
            return

        while self.fans_layout.count() < len(status['Fans']):
            widget = FanWidget()
            self.fans_layout.addWidget(widget)

        while self.fans_layout.count() > len(status['Fans']):
            widget = self.fans_layout.itemAt(self.fans_layout.count() - 1).widget()
            self.fans_layout.removeWidget(widget)
            widget.deleteLater()

        for fan_index, fan_data in enumerate(status['Fans']):
            widget = self.fans_layout.itemAt(fan_index).widget()
            widget.update(fan_index, fan_data)

class BasicConfigWidget(QWidget):
    def __init__(self):
        super().__init__()
        layout = QVBoxLayout()
        self.setLayout(layout)

        self.list_all_radio = QRadioButton("List all configurations", self)
        self.list_all_radio.clicked.connect(self.list_all_radio_checked)
        layout.addWidget(self.list_all_radio)

        self.list_recommended_radio = QRadioButton("List recommended configurations", self)
        self.list_recommended_radio.clicked.connect(self.list_recommended_radio_checked)
        layout.addWidget(self.list_recommended_radio)

        self.model_name_label = QLabel("Your laptop model: <b>%s</b>" % NBFC_CLIENT.get_model_name(), self)
        layout.addWidget(self.model_name_label)

        self.configurations_combobox = QComboBox()
        self.configurations_combobox.currentIndexChanged.connect(self.configurations_combobox_changed)
        layout.addWidget(self.configurations_combobox)

        layout.addStretch()

        self.apply_buttons_widget = ApplyButtonsWidget()
        self.apply_buttons_widget.apply_button.clicked.connect(self.apply)
        self.apply_buttons_widget.apply_with_restart_button.clicked.connect(self.apply_with_restart)
        layout.addWidget(self.apply_buttons_widget)

        self.list_all_radio.setChecked(True)
        self.list_all_radio_checked()

    def start(self):
        if not IS_ROOT:
            self.apply_buttons_widget.disable(NOT_ROOT_NOTICE)
        elif self.configurations_combobox.currentIndex() == 0:
            self.apply_buttons_widget.disable("No model configuration selected")
        else:
            self.apply_buttons_widget.enable()

    def stop(self):
        pass

    def apply(self):
        config = NBFC_CLIENT.get_config()
        old_config = config.get('SelectedConfigId', '')
        config['SelectedConfigId'] = self.configurations_combobox.currentText()
        NBFC_CLIENT.set_config(config)

        if old_config != config['SelectedConfigId']:
            MODEL_CONFIG_CHANGED.changed.emit()

    def apply_with_restart(self):
        self.apply()
        NBFC_CLIENT.restart(self.apply_buttons_widget.read_only_checkbox.isChecked())

    def update_configuration_combobox(self, configs):
        self.configurations_combobox.clear()
        self.configurations_combobox.addItem("None")
        self.configurations_combobox.addItems(configs)

        config = NBFC_CLIENT.get_config()
        if 'SelectedConfigId' in config:
            selected_config = config['SelectedConfigId']
            for i in range(self.configurations_combobox.count()):
                if self.configurations_combobox.itemText(i) == selected_config:
                    self.configurations_combobox.setCurrentIndex(i)
                    break

    def configurations_combobox_changed(self, index):
        self.start()

    def list_all_radio_checked(self):
        configs = NBFC_CLIENT.list_configs()
        self.update_configuration_combobox(configs)

    def list_recommended_radio_checked(self):
        configs = NBFC_CLIENT.recommended_configs()
        self.update_configuration_combobox(configs)

class TemperatureSourceWidget(QGroupBox):
    def __init__(self, parent):
        super().__init__()
        self.parent = parent

        layout = QVBoxLayout()
        self.setLayout(layout)

        algorithm_layout = QHBoxLayout()
        self.average_radio = QRadioButton("Average", self)
        self.max_radio = QRadioButton("Max", self)
        self.min_radio = QRadioButton("Min", self)
        algorithm_layout.addWidget(self.average_radio)
        algorithm_layout.addWidget(self.max_radio)
        algorithm_layout.addWidget(self.min_radio)
        self.average_radio.setChecked(True)
        layout.addLayout(algorithm_layout)

        self.temperature_sources = QListWidget(self)
        layout.addWidget(self.temperature_sources)

        button_layout = QHBoxLayout()
        self.add_button = QPushButton("Add", self)
        self.del_button = QPushButton("Delete", self)
        self.add_button.clicked.connect(self.add_button_clicked)
        self.del_button.clicked.connect(self.del_button_clicked)
        button_layout.addWidget(self.add_button)
        button_layout.addWidget(self.del_button)
        layout.addLayout(button_layout)

    def setFanName(self, name):
        self.setTitle(name)

    def add_button_clicked(self):
        item = self.parent.available_temperature_sources.currentItem()
        if item:
            new_item = QListWidgetItem(item.text())
            new_item.setData(Qt.UserRole, item.data(Qt.UserRole))
            self.temperature_sources.addItem(new_item)

    def del_button_clicked(self):
        self.temperature_sources.takeItem(self.temperature_sources.currentRow())


class TemperatureSourcesWidget(QStackedWidget):
    def __init__(self):
        super().__init__()

        MODEL_CONFIG_CHANGED.changed.connect(self.setup_ui)

        # Error Widget ========================================================
        self.error_widget = QWidget()
        error_layout = QVBoxLayout()
        self.error_widget.setLayout(error_layout)
        self.error_label = QLabel("", self)
        error_layout.addWidget(self.error_label)
        button_layout = QHBoxLayout()
        self.retry_button = QPushButton("Retry", self)
        self.retry_button.clicked.connect(self.setup_ui)
        self.fix_button = QPushButton("Fix errors automatically", self)
        self.fix_button.clicked.connect(self.fix_errors)
        button_layout.addWidget(self.retry_button)
        button_layout.addWidget(self.fix_button)
        error_layout.addLayout(button_layout)
        self.addWidget(self.error_widget)

        # Main Widget =========================================================
        self.main_widget = QWidget()
        main_layout = QVBoxLayout()
        self.main_widget.setLayout(main_layout)

        self.available_temperature_sources = QListWidget(self)
        self.available_temperature_sources.setMaximumHeight(100)
        main_layout.addWidget(self.available_temperature_sources)

        self.scroll_area = QScrollArea()
        self.scroll_area.setWidgetResizable(True)
        self.scroll_area.setMinimumHeight(200)
        self.scroll_area.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        temperature_sources_widget = QWidget()
        self.temperature_sources_layout = QVBoxLayout()
        temperature_sources_widget.setLayout(self.temperature_sources_layout)
        self.scroll_area.setWidget(temperature_sources_widget)
        main_layout.addWidget(self.scroll_area)

        self.apply_buttons_widget = ApplyButtonsWidget()
        self.apply_buttons_widget.apply_button.clicked.connect(self.apply)
        self.apply_buttons_widget.apply_with_restart_button.clicked.connect(self.apply_with_restart)
        main_layout.addWidget(self.apply_buttons_widget)
        self.addWidget(self.main_widget)

        self.setup_ui()

    def start(self):
        pass

    def stop(self):
        pass

    def apply(self):
        config = NBFC_CLIENT.get_config()
        config['FanTemperatureSources'] = self.get_fan_temperature_sources()
        if not len(config['FanTemperatureSources']):
            del config['FanTemperatureSources']
        NBFC_CLIENT.set_config(config)

    def apply_with_restart(self):
        self.apply()
        NBFC_CLIENT.restart(self.apply_buttons_widget.read_only_checkbox.isChecked())

    def get_model_configuration(self):
        config = NBFC_CLIENT.get_config()
        if 'SelectedConfigId' not in config:
            raise Exception('No model configuration selected')

        config_id = config['SelectedConfigId']
        model_config_path = os.path.join(NBFC_CLIENT.model_configs_dir, config_id + '.json')

        with open(model_config_path, 'r') as fh:
            return json.load(fh)

    def find_sensor_item(self, sensor):
        for i in range(self.available_temperature_sources.count()):
            item = self.available_temperature_sources.item(i)
            if item.data(Qt.UserRole) == sensor:
                return item

        raise Exception('No sensor found for %s' % sensor)

    def fix_errors(self):
        self.setup_ui(fix_errors=True)

    def setup_ui(self, fix_errors=False):
        if not IS_ROOT:
            self.apply_buttons_widget.disable(NOT_ROOT_NOTICE)
        else:
            self.apply_buttons_widget.enable()

        # =====================================================================
        # Get model configuration
        # =====================================================================
        try:
            config = NBFC_CLIENT.get_config()
            fan_temperature_sources = config.get('FanTemperatureSources', [])
            model_config = self.get_model_configuration()
        except Exception as e:
            self.setCurrentWidget(self.error_widget)
            self.error_label.setText(str(e))
            self.fix_button.setEnabled(False)
            self.retry_button.setEnabled(True)
            self.apply_buttons_widget.disable("")
            return

        # =====================================================================
        # Get available temperature sensors
        # =====================================================================
        available_sensors = get_sensors()

        # =====================================================================
        # Fill self.available_temperature_sources with available_sensors
        # =====================================================================
        self.available_temperature_sources.clear()

        sensor_names = set()
        for sensor in available_sensors:
            if sensor.name not in sensor_names:
                sensor_names.add(sensor.name)
                item = QListWidgetItem(sensor.name)
                item.setData(Qt.UserRole, sensor.name)
                self.available_temperature_sources.addItem(item)

        for sensor in available_sensors:
            item = QListWidgetItem("%s (%s)" % (sensor.file, sensor.name))
            item.setData(Qt.UserRole, sensor.file)
            self.available_temperature_sources.addItem(item)

        # =====================================================================
        # Ensure that the FanTemperatureSources in the config are valid.
        # Give the user the chance to fix it or fix it automatically.
        # =====================================================================
        errors = get_fan_temperature_sources_errors(
            fan_temperature_sources,
            len(model_config['FanConfigurations']),
            available_sensors)

        if errors and not fix_errors:
            self.setCurrentWidget(self.error_widget)
            self.error_label.setText('\n\n'.join(errors))
            self.fix_button.setEnabled(True)
            self.retry_button.setEnabled(True)
            self.apply_buttons_widget.disable("")
            return
        elif errors and fix_errors:
            fan_temperature_sources = fix_fan_temperature_sources(
                fan_temperature_sources,
                len(model_config['FanConfigurations']),
                available_sensors)

        self.setCurrentWidget(self.main_widget)

        # =====================================================================
        # Add widgets to self.temperature_sources_layout
        # =====================================================================
        while self.temperature_sources_layout.count() < len(model_config['FanConfigurations']):
            widget = TemperatureSourceWidget(self)
            self.temperature_sources_layout.addWidget(widget)

        while self.temperature_sources_layout.count() > len(model_config['FanConfigurations']):
            widget = self.temperature_sources_layout.itemAt(self.temperature_sources_layout.count() - 1).widget()
            self.temperature_sources_layout.removeWidget(widget)
            widget.deleteLater()

        # =====================================================================
        # Set fan names to widgets
        # =====================================================================
        for i, fan_config in enumerate(model_config['FanConfigurations']):
            widget = self.temperature_sources_layout.itemAt(i).widget()
            widget.setFanName(fan_config.get('FanDisplayName', 'Fan #%d' % i))

        # =====================================================================
        # Update TemperatureSourceWidget 
        # =====================================================================
        for fan_temperature_source in fan_temperature_sources:
            fan_index = fan_temperature_source['FanIndex']
            widget = self.temperature_sources_layout.itemAt(fan_index).widget()

            {
                'Average': widget.average_radio,
                'Max': widget.max_radio,
                'Min': widget.min_radio
            }[fan_temperature_source.get('TemperatureAlgorithmType', 'Average')].setChecked(True)

            widget.temperature_sources.clear()
            for sensor in fan_temperature_source.get('Sensors', []):
                item = self.find_sensor_item(sensor)
                new_item = QListWidgetItem(item.text())
                new_item.setData(Qt.UserRole, item.data(Qt.UserRole))
                widget.temperature_sources.addItem(new_item)

    def get_fan_temperature_sources(self):
        # TODO: maybe set the fan index in widget

        fan_temperature_sources = []
        for i in range(self.temperature_sources_layout.count()):
            widget = self.temperature_sources_layout.itemAt(i).widget()
            fan_json = {'FanIndex': i, 'Sensors': []}

            for j in range(widget.temperature_sources.count()):
                sensor = widget.temperature_sources.item(j).data(Qt.UserRole)
                fan_json['Sensors'].append(sensor)

            if widget.average_radio.isChecked():
                fan_json['TemperatureAlgorithmType'] = 'Average'
            elif widget.max_radio.isChecked():
                fan_json['TemperatureAlgorithmType'] = 'Max'
            elif widget.min_radio.isChecked():
                fan_json['TemperatureAlgorithmType'] = 'Min'

            # If there are no sensors, delete the key
            if not len(fan_json['Sensors']):
                del fan_json['Sensors']

            # If TemperatureAlgorithmType is 'Average' (the default), delete the key
            if fan_json['TemperatureAlgorithmType'] == 'Average':
                del fan_json['TemperatureAlgorithmType']

            # If FanTemperatureSource only has 'FanIndex', don't add it
            if len(fan_json) > 1:
                fan_temperature_sources.append(fan_json)

        return fan_temperature_sources

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("NBFC Client")
        self.resize(400, 400)

        self.tab_widget = QTabWidget(self)

        basic_config_widget = BasicConfigWidget()
        fan_control_widget = FanControlWidget()
        temperature_sources_widget = TemperatureSourcesWidget()

        self.tab_widget.addTab(fan_control_widget, "Fans")
        self.tab_widget.addTab(basic_config_widget, "Basic Configuration")
        self.tab_widget.addTab(temperature_sources_widget, "Temperature Sources")

        self.tab_widget.currentChanged.connect(self.tabChanged)
        self.tabChanged(0)

        self.setCentralWidget(self.tab_widget)

        menuBar = self.menuBar()
        applicationMenu = menuBar.addMenu("&Application")
        quitAction = QAction("&Quit", self)
        quitAction.setShortcut("Ctrl+Q")
        quitAction.triggered.connect(lambda: QApplication.quit())
        applicationMenu.addAction(quitAction)
        aboutAction = QAction("&About", self)
        aboutAction.triggered.connect(self.showAbout)
        applicationMenu.addAction(aboutAction)

    def showAbout(self):
        QMessageBox.about(self, "About NBFC-Linux", ABOUT_NBFC_LINUX_QT)

    def tabChanged(self, current_index):
        for i in range(self.tab_widget.count()):
            widget = self.tab_widget.widget(i)
            if i == current_index:
                widget.start()
            else:
                widget.stop()

if __name__ == '__main__':
    # Make CTLR+C work
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    app = QApplication(sys.argv)
    main_window = MainWindow()
    main_window.show()
    sys.exit(app.exec_())
