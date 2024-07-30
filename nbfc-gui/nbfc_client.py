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

