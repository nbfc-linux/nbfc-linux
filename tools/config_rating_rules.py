#!/usr/bin/env python3

import sys
import json

RULES = {
  "FanRegisterFullMatch": [
      {
          "Name": "PFAN",
          "Mode": "rw",
          "WritePriority": 100
      },
      {
          "Name": "SFAN",
          "Mode": "rw"
      },
      {
          "Name": "CFAN",
          "Mode": "rw",
          "ReadPriority": 100
      },
      {
          "Name": "XFAN",
          "Mode": "rw"
      },
      {
          "Name": "FAN1",
          "Mode": "rw"
      },
      {
          "Name": "FSW1",
          "Mode": "rw",
          "WritePriority": 100
      },
      {
          "Name": "RTMP",
          "Mode": "rw",
          "Notice": "Temperature register; may cause CPU throttling or emergency shutdown"
      },
      {
          "Name": "GPUF",
          "Mode": "rw"
      },
      {
          "Name": "CPUF",
          "Mode": "rw"
      },
      {
          "Name": "C1FR",
          "Mode": "r"
      },
      {
          "Name": "G1FR",
          "Mode": "r"
      },
      {
          "Name": "FRDC",
          "Mode": "r",
          "ReadPriority": 100
      },
      {
          "Name": "FTGC",
          "Mode": "w",
          "WritePriority": 100
      },
      {
          "Name": "FR2C",
          "Mode": "r",
          "ReadPriority": 100
      },
      {
          "Name": "FT2C",
          "Mode": "w",
          "WritePriority": 100
      },
  ],

  "FanRegisterPartialMatch": [
      "FAN",
      "RPM",
      "PWM"
   ],

  "RegisterWriteFullMatch": [
      "TEMP",
      "CRZN",
      "FSH1",
      "FASO"
  ],

  "RegisterWritePartialMatch": [
  ],

  "BadRegisterFullMatch": [
      "FBCM",
      "FBGI",
      "FBAE",
      "FBCB",
      "FBW1",
      "FBW2",
      "FBID",
      "FUAE",
      "FRPS"
  ],

  "BadRegisterPartialMatch": [
      "BAT"
  ]
}

def make_v1_FanRegisterFullMatch(match):
    match = match.copy()

    if 'ReadPriority' in match:
        match.pop('ReadPriority')

    if 'WritePriority' in match:
        match.pop('WritePriority')

    if 'Notice' in match:
        match.pop('Notice')

    return match

def make_v1_rules(rules):
    rules = rules.copy()

    rules['FanRegisterFullMatch'] = list(map(
        make_v1_FanRegisterFullMatch,
        rules['FanRegisterFullMatch']
    ))

    return rules

if sys.argv[1] == 'v1':
    print(json.dumps(make_v1_rules(RULES), indent=2))
elif sys.argv[1] == 'v2':
    print(json.dumps(RULES, indent=2))
elif sys.argv[1] == 'c_code':
    text = json.dumps(RULES, indent=None, separators=(',', ':'))
    ctext = text.replace('"', '\\"')

    print('#ifndef NBFC_CONFIG_RATING_RULES_DEFAULT_H_')
    print('#define NBFC_CONFIG_RATING_RULES_DEFAULT_H_')
    print('')
    print('/* Auto generated code %r */' % sys.argv)
    print('')
    print('#define CONFIG_RATING_DEFAULT_RULES \\')
    print(f'"{ctext}"')
    print('')
    print('#endif')
