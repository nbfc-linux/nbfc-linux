#!/usr/bin/env python3

import os
import pathlib

os.chdir(pathlib.Path(__file__).resolve().parent)

json_files = []

for root, dirs, files in os.walk('.'):
    for file in files:
        if file.endswith('.json'):
            json_files.append('%s/%s' % (root, file))

json_files = sorted(json_files)

with open('Makefile', 'w') as fh:
    print('all: test_config', file=fh)
    for file in json_files:
        print(f'\t./test_config {file}', file=fh)
    print('', file=fh)
    print('test_config: test_config.c', file=fh)
    print('\t$(CC) -O2 test_config.c -o test_config', file=fh)
