#!/usr/bin/python

import re
import os
import sys

if sys.argv[0] != './include_files.py':
    raise Exception("Please run this program from the directory containing it")

def include_files(file):
    with open(file, 'r') as fh:
        lines = fh.readlines()
        for line in lines:
            m = re.match('.*# *include (\\S+)', line)
            if m:
                include_files(m[1])
            else:
                print(line, end='')

include_files(sys.argv[1])

