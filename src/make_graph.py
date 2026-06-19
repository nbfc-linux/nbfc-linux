#!/usr/bin/env python3

import sys
import math
import argparse

try:
    import matplotlib.pyplot as plot
except ImportError:
    print('Please install the Python matplotlib library\n', file=sys.stderr)
    print(' Arch:   sudo pacman -S python-matplotlib',      file=sys.stderr)
    print(' Debian: sudo apt install python3-matplotlib',   file=sys.stderr)
    print(' Fedora: sudo dnf install python3-matplotlib',   file=sys.stderr)
    sys.exit(1)

def make_graphs(data):
    num = len(data)
    width = math.ceil(math.sqrt(num))
    height = math.ceil(math.sqrt(num))

    fig, axs = plot.subplots(height, width)

    for i, (reg, values) in enumerate(data.items()):
        row = i // width
        col = i % width
        axs[row, col].plot(values)
        axs[row, col].set_title(reg)

    fig.subplots_adjust(
        left=None, bottom=None, right=None, top=None, wspace=1, hspace=1
    )

    plot.show()

def read_dump(filename, decimal=False):
    if decimal:
        base = 10
    else:
        base = 16

    with open(filename, "r") as fh:
        data = {}
        for line in fh:
            buff = line.strip().split(",")
            reg = buff.pop(0)
            values = [int(x, base) for x in buff]
            data[reg] = values

    return data

argp = argparse.ArgumentParser()
argp.add_argument('file')
argp.add_argument('-d', '--decimal', action='store_true')
opts = argp.parse_args()

data = read_dump(opts.file, opts.decimal)
make_graphs(data)
