#!/usr/bin/python3

from time import sleep

from nbfc_client import *

NBFC_CLIENT = NbfcClient()

try:
    NBFC_CLIENT.start()
except Exception as e:
    print("NBFC_CLIENT.start(): %s" % e)

sleep(1)

try:
    NBFC_CLIENT.restart()
except Exception as e:
    print("NBFC_CLIENT.restart(): %s" % e)

try:
    status = NBFC_CLIENT.get_status()
    print("Status: %s" % status)
except Exception as e:
    print("NBFC_CLIENT.get_status(): %s" % e)

try:
    response = NBFC_CLIENT.list_configs()
    print("List configs: %s" % response[0:10])
except Exception as e:
    print("NBFC_CLIENT.list_configs(): %s" % e)

try:
    response = NBFC_CLIENT.recommended_configs()
    print("Recommended configs: %s" % response[0:10])
except Exception as e:
    print("NBFC_CLIENT.recommended_configs(): %s" % e)

try:
    NBFC_CLIENT.set_fan_speed(0)
except Exception as e:
    print("NBFC_CLIENT.set_fan_speed(0): %s" % e)

try:
    NBFC_CLIENT.set_fan_speed('auto')
except Exception as e:
    print("NBFC_CLIENT.set_fan_speed('auto'): %s" % e)

try:
    NBFC_CLIENT.stop()
except Exception as e:
    print("NBFC_CLIENT.stop(): %s" % e)
