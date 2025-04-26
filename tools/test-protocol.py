#!/usr/bin/python3

import json
import socket

SOCKET = '/var/run/nbfc_service.socket'
MAX_MESSAGE_SIZE = 256

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

# General testing =============================================================
do_test('',
        {'Error': 'Invalid JSON: Unexpected end of text'})

do_test('1',
        {'Error': 'Not a JSON object'})

do_test('{}',
        {'Error': "Missing 'Command' field"})

do_test('{"Command": 1}',
        {'Error': 'Command: Not a string'})

do_test('{"Command": "foo"}',
        {'Error': 'Invalid command'})

do_test('x' * (MAX_MESSAGE_SIZE + 1),
        {'Error': 'Message too large'})

# Command "set-fan-speed" =====================================================
do_test('{"Command": "set-fan-speed"}',
        {'Error': 'Missing argument: Speed'})

do_test('{"Command": "set-fan-speed", "invalid_arg": 1}',
        {'Error': 'Unknown arguments'})

do_test('{"Command": "set-fan-speed", "Speed": -1}',
        {'Error': 'Speed: Invalid value'})

do_test('{"Command": "set-fan-speed", "Speed": 101}',
        {'Error': 'Speed: Invalid value'})

do_test('{"Command": "set-fan-speed", "Speed": "auto"}',
        {'Status': 'OK'})

do_test('{"Command": "set-fan-speed", "Speed": 0}',
        {'Status': 'OK'})

do_test('{"Command": "set-fan-speed", "Speed": 1}',
        {'Status': 'OK'})

do_test('{"Command": "set-fan-speed", "Speed": 100}',
        {'Status': 'OK'})

do_test('{"Command": "set-fan-speed", "Speed": 100.0}',
        {'Status': 'OK'})

do_test('{"Command": "set-fan-speed", "Speed": 100.0, "Fan": 0}',
        {'Status': 'OK'})

do_test('{"Command": "set-fan-speed", "Speed": 100.0, "Fan": 1.0}',
        {'Error': 'Fan: Not an integer'})

do_test('{"Command": "set-fan-speed", "Speed": 100.0, "Fan": "1"}',
        {'Error': 'Fan: Not an integer'})

do_test('{"Command": "set-fan-speed", "Speed": 100.0, "Fan": 100}',
        {'Error': 'Fan: No such fan available'})

do_test('{"Command": "set-fan-speed", "Speed": 100000000000000000000000}',
        {'Error': 'Invalid JSON: Invalid number'})

# Command "status" ============================================================
do_test('{"Command": "status"}')

do_test('{"Command": "status", "invalid_arg": 1}',
        {'Error': 'Unknown arguments'})
