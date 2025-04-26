#!/usr/bin/env python3

import socket

SOCKET = '/var/run/nbfc_service.socket'

def stress_parallel_connections(num_connections):
    sockets = []

    for i in range(num_connections):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(SOCKET)
        sockets.append(sock)

    for sock in sockets:
        sock.close()

stress_parallel_connections(1000)
