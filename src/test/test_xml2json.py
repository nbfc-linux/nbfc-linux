#!/usr/bin/python3

"""
The `nbfc xml2json` command replaces the `config_to_json.py` script.

This test ensures that `nbfc xml2json` produces the same output
as `config_to_json.py`.
"""

import os
import json
import glob
import shutil
import subprocess
import deepdiff

# Script configuration
XML_DIR        = '../../xml'
TMP_DIR        = '/tmp/nbfc-linux-xml2json-test'
NBFC           = '../nbfc'
CONFIG_TO_JSON = '../../tools/config_to_json.py'

# Ignore these differences while testing
IGNORE = {
    'dictionary_item_removed': [
        "root['FanConfigurations'][0]['FanDisplayName']",
        "root['Author']"
    ]
}

os.chdir(os.path.dirname(os.path.abspath(__file__)))
os.makedirs(TMP_DIR, exist_ok=True)

def run(args):
    return subprocess.run(
      args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

def round_floats(obj, digits=2):
    if isinstance(obj, float):
        return round(obj, digits)

    if isinstance(obj, dict):
        return { k: round_floats(v, digits) for k, v in obj.items() }

    if isinstance(obj, list):
        return [ round_floats(v, digits) for v in obj ]

    if isinstance(obj, tuple):
        return tuple( round_floats(v, digits) for v in obj )

    return obj

def diff_configs(original, converted):
    diff = deepdiff.DeepDiff(original, converted)

    for key, values in IGNORE.items():
        if key in diff:
            for value in values:
                try:
                    diff[key].remove(value)
                except KeyError:
                    pass

    for key in list(diff.keys()):
        if not diff[key]:
            diff.pop(key)

    return diff

def test(file_path):
    file_name    = os.path.basename(file_path)
    file_base, _ = os.path.splitext(file_name)
    temp_xml     = "%s/%s" % (TMP_DIR, file_name)
    temp_json    = "%s/%s.json" % (TMP_DIR, file_base)

    # Copy XML file to temp
    shutil.copy(file_path, temp_xml)

    # Run `config_to_json.py` on temp xml file
    run([CONFIG_TO_JSON, '--out-dir', TMP_DIR, temp_xml])
    with open(temp_json, 'r') as fh:
        original_json = json.load(fh)

    # Run `nbfc xml2json` on temp xml file
    r = run([NBFC, 'xml2json', temp_xml])
    converted_json = json.loads(r.stdout)

    # Diff
    converted_json = round_floats(converted_json, 3)
    original_json  = round_floats(original_json, 3)
    diff = diff_configs(original_json, converted_json)
    if diff:
        raise Exception(diff)

for file_path in glob.glob(XML_DIR + '/*.xml'):
    try:
        print(file_path)
        test(file_path)
    except Exception as e:
        print('Failed:', file_path, e)
