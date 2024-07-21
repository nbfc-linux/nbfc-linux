#!/usr/bin/python3

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GLib

from about import *       # include about.py
from common import *      # include common.py
from fs_sensors import *  # include fs_sensors.py
from nbfc_client import * # include nbfc_client.py

IS_ROOT = (os.geteuid() == 0)
NBFC_CLIENT = NbfcClient()
NOT_ROOT_NOTICE = "You cannot change the configuration because you are not root"

class ApplyButtonsWidget(Gtk.VBox):
    def __init__(self):
        super().__init__()

        self.error_label = Gtk.Label()
        self.add(self.error_label)

        self.read_only_checkbox = Gtk.CheckButton(label="(Re-)start in read-only mode")
        self.add(self.read_only_checkbox)

        hbox = Gtk.HBox()
        self.add(hbox)

        self.apply_button = Gtk.Button(label="Apply")
        hbox.add(self.apply_button)

        self.apply_with_restart_button = Gtk.Button(label="Apply with (re-)start")
        hbox.add(self.apply_with_restart_button)

    def enable(self):
        self.error_label.hide()
        self.read_only_checkbox.set_sensitive(True)
        self.apply_button.set_sensitive(True)
        self.apply_with_restart_button.set_sensitive(True)

    def disable(self, reason):
        self.error_label.show()
        self.error_label.set_text(reason)
        self.read_only_checkbox.set_sensitive(False)
        self.apply_button.set_sensitive(False)
        self.apply_with_restart_button.set_sensitive(False)

class FanWidget(Gtk.Frame):
    def __init__(self):
        super().__init__()

        self.fan_index = 0

        # Main layout =========================================================
        main_layout = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.add(main_layout)

        # Grid box ============================================================
        grid_box = Gtk.Grid()
        main_layout.add(grid_box)

        label = Gtk.Label(label="Fan name")
        self.name_label = Gtk.Label()
        grid_box.attach(label, 0, 0, 1, 1)
        grid_box.attach(self.name_label, 1, 0, 1, 1)

        label = Gtk.Label(label="Temperature")
        self.temperature_label = Gtk.Label()
        grid_box.attach(label, 0, 1, 1, 1)
        grid_box.attach(self.temperature_label, 1, 1, 1, 1)

        label = Gtk.Label(label="Auto mode")
        self.auto_mode_label = Gtk.Label()
        grid_box.attach(label, 0, 2, 1, 1)
        grid_box.attach(self.auto_mode_label, 1, 2, 1, 1)

        label = Gtk.Label(label="Critical")
        self.critical_label = Gtk.Label()
        grid_box.attach(label, 0, 3, 1, 1)
        grid_box.attach(self.critical_label, 1, 3, 1, 1)

        label = Gtk.Label(label="Current speed")
        self.current_speed_label = Gtk.Label()
        grid_box.attach(label, 0, 4, 1, 1)
        grid_box.attach(self.current_speed_label, 1, 4, 1, 1)

        label = Gtk.Label(label="Target speed")
        self.target_speed_label = Gtk.Label()
        grid_box.attach(label, 0, 5, 1, 1)
        grid_box.attach(self.target_speed_label, 1, 5, 1, 1)

        label = Gtk.Label(label="Speed steps")
        self.speed_steps_label = Gtk.Label()
        grid_box.attach(label, 0, 6, 1, 1)
        grid_box.attach(self.speed_steps_label, 1, 6, 1, 1)

        # Auto mode checkbox ==================================================
        self.auto_mode_checkbox = Gtk.CheckButton(label="Auto mode")
        self.auto_mode_checkbox.connect("toggled", self.update_fan_speed)
        main_layout.add(self.auto_mode_checkbox)
        
        # Speed slider ========================================================
        adjustment = Gtk.Adjustment(value=0, lower=0, upper=100, step_increment=1, page_increment=10, page_size=0)
        self.speed_scale = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL, adjustment=adjustment)
        self.speed_scale.set_hexpand(True)
        self.speed_scale.set_value_pos(Gtk.PositionType.RIGHT)
        self.speed_scale.set_digits(0)
        self.speed_scale.connect("value-changed", self.update_fan_speed)
        main_layout.add(self.speed_scale)

    def update_fan_speed(self, *_):
        if self.auto_mode_checkbox.get_active():
            NBFC_CLIENT.set_fan_speed('auto', self.fan_index)
        else:
            NBFC_CLIENT.set_fan_speed(self.speed_scale.get_value(), self.fan_index)

    def update(self, fan_index, fan_data):
        self.fan_index = fan_index
        self.name_label.set_text(fan_data['Name'])
        self.temperature_label.set_text(str(fan_data['Temperature']))
        self.auto_mode_label.set_text(str(fan_data['AutoMode']))
        self.critical_label.set_text(str(fan_data['Critical']))
        self.current_speed_label.set_text(str(fan_data['CurrentSpeed']))
        self.target_speed_label.set_text(str(fan_data['TargetSpeed']))
        self.speed_steps_label.set_text(str(fan_data['SpeedSteps']))

        # Block signals to avoid triggering during update
        self.auto_mode_checkbox.handler_block_by_func(self.update_fan_speed)
        self.speed_scale.handler_block_by_func(self.update_fan_speed)

        self.auto_mode_checkbox.set_active(fan_data['AutoMode'])
        self.speed_scale.set_value(int(fan_data['RequestedSpeed']))

        # Unblock signals after update
        self.auto_mode_checkbox.handler_unblock_by_func(self.update_fan_speed)
        self.speed_scale.handler_unblock_by_func(self.update_fan_speed)


class FansWidget(Gtk.Frame):
    def __init__(self):
        super().__init__()
        self.timer_id = None

        self.stack = Gtk.Stack()
        self.add(self.stack)

        self.error_layout = Gtk.VBox()
        self.error_label = Gtk.Label()
        self.error_layout.pack_start(self.error_label, True, True, 0)
        self.stack.add_titled(self.error_layout, "error_layout", "Error")

        self.scroll_window = Gtk.ScrolledWindow()
        self.fans_layout = Gtk.VBox()
        self.scroll_window.add(self.fans_layout)
        self.stack.add_titled(self.scroll_window, "scroll_window", "Fans")

    def start(self):
        if self.timer_id is None:
            self.timer_id = GLib.timeout_add(500, self.update)

    def stop(self):
        if self.timer_id is not None:
            GLib.source_remove(self.timer_id)
            self.timer_id = None

    def update(self):
        try:
            status = NBFC_CLIENT.get_status()
            self.stack.set_visible_child(self.scroll_window)
        except Exception as e:
            self.stack.set_visible_child(self.error_layout)
            self.error_label.set_text(str(e))
            return True

        # Add widgets
        while len(self.fans_layout.get_children()) < len(status['Fans']):
            widget = FanWidget()
            widget.show_all()
            self.fans_layout.pack_start(widget, True, True, 0)

        # Remove widgets
        for i, child in enumerate(self.fans_layout.get_children()):
            if i >= len(status['Fans']):
                self.fans_layout.remove(child)

        # Update widget data
        fan_index = 0
        for fan_data, widget in zip(status['Fans'], self.fans_layout.get_children()):
            widget.update(fan_index, fan_data)
            fan_index += 1

        return True

class BasicConfigWidget(Gtk.Frame):
    def __init__(self):
        super().__init__()

        vbox = Gtk.VBox()
        self.add(vbox)
        
        # Radio Buttons =======================================================
        self.radio_config_all = Gtk.RadioButton(label="List all configurations")
        self.radio_config_all.connect("clicked", self.radio_config_all_clicked)
        vbox.add(self.radio_config_all)

        self.radio_config_recommended = Gtk.RadioButton(label="List recommended configurations")
        self.radio_config_recommended.join_group(self.radio_config_all)
        self.radio_config_recommended.connect("clicked", self.radio_config_recommended_clicked)
        vbox.add(self.radio_config_recommended)

        ## Configuration ComboBox ==============================================
        self.configurations = Gtk.ComboBoxText()
        self.configurations.set_entry_text_column(0)
        self.configurations.connect("changed", self.configurations_changed)
        vbox.add(self.configurations)

        self.apply_buttons_widget = ApplyButtonsWidget()
        self.apply_buttons_widget.apply_button.connect("clicked", self.apply)
        self.apply_buttons_widget.apply_with_restart_button.connect("clicked", self.apply_with_restart)
        vbox.pack_start(self.apply_buttons_widget, False, False, 0)

        # Init
        self.radio_config_all.set_active(True)
        self.radio_config_all_clicked()

    def start(self):
        if not IS_ROOT:
            self.apply_buttons_widget.disable(NOT_ROOT_NOTICE)
        elif self.configurations.get_active() <= 0:
            self.apply_buttons_widget.disable("No model configuration selected")
        else:
            self.apply_buttons_widget.enable()

    def stop(self):
        pass

    def apply(self, *_):
        config = NBFC_CLIENT.get_config()
        old_config = config.get('SelectedConfigId', '')
        config['SelectedConfigId'] = self.configurations_combobox.currentText()
        NBFC_CLIENT.set_config(config)

        # TODO
        #if old_config != config['SelectedConfigId']:
        #    MODEL_CONFIG_CHANGED.changed.emit()

    def apply_with_restart(self, *_):
        self.apply()
        NBFC_CLIENT.restart(self.apply_buttons_widget.read_only_checkbox.isChecked())

    def configurations_changed(self, *_):
        self.start()

    def update_configuration_combobox(self, configs):
        self.configurations.get_model().clear()
        self.configurations.append_text('None')

        for config in configs:
            self.configurations.append_text(config)

        self.configurations.set_active(0)

        config = NBFC_CLIENT.get_config()
        if 'SelectedConfigId' in config:
            current_config = config['SelectedConfigId']

            model = self.configurations.get_model()
            iterator = model.get_iter_first()
            index = 0
            while iterator:
                if model[iterator][0] == current_config:
                    self.configurations.set_active(index)
                    break

                iterator = model.iter_next(iterator)
                index += 1

    def radio_config_all_clicked(self, *_):
        configs = NBFC_CLIENT.list_configs()
        self.update_configuration_combobox(configs)

    def radio_config_recommended_clicked(self, *_):
        configs = NBFC_CLIENT.recommended_configs()
        self.update_configuration_combobox(configs)

class TemperatureSourceWidget(Gtk.Frame):
    def __init__(self, parent):
        super().__init__()
        self.parent = parent

        vbox = Gtk.VBox()
        self.add(vbox)

        self.fan_label = Gtk.Label()
        vbox.add(self.fan_label)

        algorithm_layout = Gtk.HBox()
        self.average_radio = Gtk.RadioButton(label="Average")
        self.max_radio = Gtk.RadioButton(label="Max")
        self.max_radio.join_group(self.average_radio)
        self.min_radio = Gtk.RadioButton(label="Min")
        self.min_radio.join_group(self.average_radio)
        algorithm_layout.add(self.average_radio)
        algorithm_layout.add(self.max_radio)
        algorithm_layout.add(self.min_radio)
        #self.average_radio.setChecked(True)
        vbox.add(algorithm_layout)

        self.temperature_sources = Gtk.ListBox()
        vbox.add(self.temperature_sources)

        button_layout = Gtk.HBox()
        self.add_button = Gtk.Button(label="Add")
        self.del_button = Gtk.Button(label="Delete")
        self.add_button.connect("clicked", self.add_button_clicked)
        self.del_button.connect("clicked", self.del_button_clicked)
        button_layout.add(self.add_button)
        button_layout.add(self.del_button)
        vbox.add(button_layout)

    def setFanName(self, name):
        self.fan_label.set_text(name)

    def add_button_clicked(self, *_):
        row = self.parent.available_temperature_sources.get_selected_row()
        if row:
            widget = row.get_child()
            new_widget = Gtk.Label(label=widget.get_text())
            setattr(new_widget, 'extra_data', getattr(widget, 'extra_data'))
            new_widget.show()
            self.temperature_sources.add(new_widget)

    def del_button_clicked(self, *_):
        row = self.temperature_sources.get_selected_row()
        widget = row.get_child()
        self.temperature_sources.remove(widget)

class TemperatureSourcesWidget(Gtk.Frame):
    def __init__(self):
        super().__init__()

        self.stack = Gtk.Stack()
        self.add(self.stack)

        # Error Widget ========================================================
        self.error_widget = Gtk.VBox()
        self.error_label = Gtk.Label()
        self.error_widget.add(self.error_label)

        button_layout = Gtk.HBox()
        self.retry_button = Gtk.Button(label="Retry")
        self.retry_button.connect("clicked", self.setup_ui)
        self.fix_button = Gtk.Button(label="Fix errors automatically")
        self.fix_button.connect("clicked", self.fix_errors)
        button_layout.add(self.retry_button)
        button_layout.add(self.fix_button)
        self.error_widget.add(button_layout)

        self.stack.add_titled(self.error_widget, "error_layout", "Error")

        # Main Widget =========================================================
        self.main_widget = Gtk.VBox()

        scroll_area = Gtk.ScrolledWindow()
        self.available_temperature_sources = Gtk.ListBox()
        self.available_temperature_sources.set_selection_mode(Gtk.SelectionMode.SINGLE)
        # TODO: add ScrolledWindow so available_temperature_sources is scrollable!
        #self.available_temperature_sources.setMaximumHeight(100)
        scroll_area.add(self.available_temperature_sources)
        self.main_widget.add(scroll_area)

        scroll_area = Gtk.ScrolledWindow()
        #self.scroll_area.setWidgetResizable(True)
        #self.scroll_area.setMinimumHeight(200)
        #self.scroll_area.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.temperature_sources_widget = Gtk.VBox()
        scroll_area.add(self.temperature_sources_widget)
        self.main_widget.add(scroll_area)

        self.apply_buttons_widget = ApplyButtonsWidget()
        self.apply_buttons_widget.apply_button.connect("clicked", self.apply)
        self.apply_buttons_widget.apply_with_restart_button.connect("clicked", self.apply_with_restart)
        self.main_widget.pack_start(self.apply_buttons_widget, False, False, 0)

        self.stack.add_titled(self.main_widget, "main_widget", "Main")

        self.stack.show_all()

        self.setup_ui()

    def start(self):
        pass

    def stop(self):
        pass

    def setup_ui(self, *_, fix_errors=False):
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
            model_config = NBFC_CLIENT.get_model_configuration()
        except Exception as e:
            self.stack.set_visible_child(self.error_widget)
            self.error_label.set_text(str(e))
            self.fix_button.set_sensitive(False)
            self.retry_button.set_sensitive(True)
            self.apply_buttons_widget.disable("")
            return

        # =====================================================================
        # Get available temperature sensors
        # =====================================================================
        available_sensors = get_sensors()

        # =====================================================================
        # Fill self.available_temperature_sources with available_sensors
        # =====================================================================
        for child in self.available_temperature_sources.get_children():
            self.available_temperature_sources.remove(child)

        sensor_names = set()
        for sensor in available_sensors:
            if sensor.name not in sensor_names:
                sensor_names.add(sensor.name)
                widget = Gtk.Label(label=sensor.name)
                setattr(widget,"extra_data", sensor.name)
                self.available_temperature_sources.add(widget)

        for sensor in available_sensors:
            widget = Gtk.Label(label="%s (%s)" % (sensor.file, sensor.name))
            setattr(widget, "extra_data", sensor.file)
            self.available_temperature_sources.add(widget)

        # =====================================================================
        # Ensure that the FanTemperatureSources in the config are valid.
        # Give the user the chance to fix it or fix it automatically.
        # =====================================================================
        errors = get_fan_temperature_sources_errors(
            fan_temperature_sources,
            len(model_config['FanConfigurations']),
            available_sensors)

        if errors and not fix_errors:
            self.stack.set_visible_child(self.error_widget)
            self.error_label.set_text('\n\n'.join(errors))
            self.fix_button.set_sensitive(True)
            self.retry_button.set_sensitive(True)
            self.apply_buttons_widget.disable("")
            return
        elif errors and fix_errors:
            fan_temperature_sources = fix_fan_temperature_sources(
                fan_temperature_sources,
                len(model_config['FanConfigurations']),
                available_sensors)

        self.stack.set_visible_child(self.main_widget)

        # =====================================================================
        # Add widgets to self.temperature_sources_layout
        # =====================================================================
        while len(self.temperature_sources_widget.get_children()) < len(model_config['FanConfigurations']):
            widget = TemperatureSourceWidget(self)
            self.temperature_sources_widget.add(widget)

        while len(self.temperature_sources_widget.get_children()) > len(model_config['FanConfigurations']):
            l = len(self.temperature_sources_widget.get_children())
            widget = self.temperature_sources_widget.get_children()[l - 1]
            self.temperature_sources_widget.remove(widget)

        # =====================================================================
        # Set fan names to widgets
        # =====================================================================
        for i, fan_config in enumerate(model_config['FanConfigurations']):
            widget = self.temperature_sources_widget.get_children()[i]
            widget.setFanName(fan_config.get('FanDisplayName', 'Fan #%d' % i))

        # =====================================================================
        # Update TemperatureSourceWidget 
        # =====================================================================
        for fan_temperature_source in fan_temperature_sources:
            fan_index = fan_temperature_source['FanIndex']
            widget = self.temperature_sources_widget.get_children()[i]

            {
                'Average': widget.average_radio,
                'Max': widget.max_radio,
                'Min': widget.min_radio
            }[fan_temperature_source.get('TemperatureAlgorithmType', 'Average')].set_active(True)

            for child in widget.temperature_sources.get_children():
                widget.temperature_sources.remove(child)

            for sensor in fan_temperature_source.get('Sensors', []):
                sensor_widget = self.find_sensor_widget(sensor)
                new_widget = Gtk.Label(label=sensor_widget.get_text())
                setattr(new_widget, 'extra_data', getattr(sensor_widget, 'extra_data'))
                widget.temperature_sources.add(new_widget)

    def fix_errors(self, *_):
        self.setup_ui(fix_errors=True)

    def apply(self, *_):
        config = NBFC_CLIENT.get_config()
        config['FanTemperatureSources'] = self.get_fan_temperature_sources()
        if not len(config['FanTemperatureSources']):
            del config['FanTemperatureSources']
        NBFC_CLIENT.set_config(config)

    def apply_with_restart(self, *_):
        self.apply()
        NBFC_CLIENT.restart(self.apply_buttons_widget.read_only_checkbox.isChecked())

    def find_sensor_widget(self, sensor):
        for widget in self.available_temperature_sources.get_children():
            if getattr(widget, 'extra_data') == sensor:
                return widget

        raise Exception('No sensor found for %s' % sensor)

    def get_fan_temperature_sources(self):
        # TODO: maybe set the fan index in widget

        fan_temperature_sources = []
        for i, widget in enumerate(self.temperature_sources_widget.get_children()):
            fan_json = {'FanIndex': i, 'Sensors': []}

            for row in widget.temperature_sources.get_children():
                temperature_widget = row.get_child()
                sensor = getattr(temperature_widget, 'extra_data')
                fan_json['Sensors'].append(sensor)

            if widget.average_radio.get_active():
                fan_json['TemperatureAlgorithmType'] = 'Average'
            elif widget.max_radio.get_active():
                fan_json['TemperatureAlgorithmType'] = 'Max'
            elif widget.min_radio.get_active():
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

class MainWindow(Gtk.Window):
    def __init__(self):
        super().__init__(title="NBFC Client")

        self.set_default_size(400, 400)

        vbox = Gtk.VBox()
        self.add(vbox)

        # Menu
        menubar = Gtk.MenuBar()

        # Application Menu
        app_menu = Gtk.Menu()
        app_menu_item = Gtk.MenuItem(label="Application")
        app_menu_item.set_submenu(app_menu)
        menubar.append(app_menu_item)

        # Quit
        quit_item = Gtk.MenuItem(label="Quit")
        quit_item.connect("activate", lambda *_: Gtk.main_quit())
        app_menu.append(quit_item)

        # About
        about_item = Gtk.MenuItem(label="About")
        about_item.connect("activate", self.showAbout)
        app_menu.append(about_item)

        vbox.pack_start(menubar, False, False, 0)

        fans_widget = FansWidget()
        basic_config_widget = BasicConfigWidget()
        temperature_sources_widget = TemperatureSourcesWidget()

        notebook = Gtk.Notebook()
        notebook.connect('switch-page', self.tabChanged)
        notebook.append_page(fans_widget, Gtk.Label(label="Fans"))
        notebook.append_page(basic_config_widget, Gtk.Label(label="Basic Configuration"))
        notebook.append_page(temperature_sources_widget, Gtk.Label(label="Temperature Sources"))

        vbox.pack_start(notebook, True, True, 0)

    def tabChanged(self, notebook, tab, index):
        for widget in notebook.get_children():
            if widget == tab:
                widget.start()
            else:
                widget.stop()

    def showAbout(self, _):
        dialog = Gtk.Dialog(title="About NBFC-Linux", parent=self)
        dialog.add_buttons(Gtk.STOCK_OK, Gtk.ResponseType.OK)
        label = Gtk.Label()
        label.set_markup(ABOUT_NBFC_LINUX_GTK)
        box = dialog.get_content_area()
        box.add(label)
        label.show()
        dialog.run()
        dialog.destroy()

if __name__ == '__main__':
    window = MainWindow()
    window.connect("destroy", Gtk.main_quit)
    window.show_all()
    Gtk.main()
