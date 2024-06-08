#!/usr/bin/python3

import json
import socket

SOCKET = '/var/run/nbfc_service.socket'

def communicate(socket_file, data):
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client_socket:
        client_socket.connect(socket_file)

        if isinstance(data, str):
            pass
        else:
            data = json.dumps(data)

        message = "%s\nEND" % data
        client_socket.sendall(message.encode('utf-8'))

        response = b''
        while True:
            data = client_socket.recv(1024)
            response += data
            if b'\nEND' in response:
                break
        
        response = response.decode('utf-8')
        response = response.replace('\nEND', '')
        response = json.loads(response)
        return response

def do_test(data, expected=None):
    response = communicate(SOCKET, data)
    if expected is not None and response != expected:
        raise Exception("Test '%s' failed. Expected: %s, Got: %s" % (
            data, expected, response))

do_test('',
        {'error': 'Invalid JSON: Unexpected end of text'})

do_test('1',
        {'error': 'Not a JSON object'})

do_test('{}',
        {'error': "Missing 'command' field"})

do_test('{"command": 1}',
        {'error': 'command: not a string'})

do_test('{"command": "foo"}',
        {'error': 'Invalid command'})

do_test('{"command": "set-fan-speed"}',
        {'error': 'Missing argument: speed'})

do_test('{"command": "set-fan-speed", "invalid_arg": 1}',
        {'error': 'Unknown arguments'})

do_test('{"command": "set-fan-speed", "speed": -1}',
        {'error': 'speed: Invalid value'})

do_test('{"command": "set-fan-speed", "speed": 101}',
        {'error': 'speed: Invalid value'})

do_test('{"command": "set-fan-speed", "speed": "auto"}',
        {'status': 'OK'})

do_test('{"command": "set-fan-speed", "speed": 0}',
        {'status': 'OK'})

do_test('{"command": "set-fan-speed", "speed": 1}',
        {'status': 'OK'})

do_test('{"command": "set-fan-speed", "speed": 100}',
        {'status': 'OK'})

do_test('{"command": "set-fan-speed", "speed": 100.0}',
        {'status': 'OK'})

do_test('{"command": "status"}')

do_test('{"command": "status", "invalid_arg": 1}',
        {'error': 'Unknown arguments'})
