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

def print_clearly_note():
    print('NOTE: This script does not support reports created with '
          '`ec_probe monitor -c|--clearly`', file=sys.stderr)

class Data:
    def __init__(self):
        self.register_readings = {}
        self.cpu_readings = []
        self.gpu_readings = []

    def validate_or_die(self):
        length = None

        for values in self.register_readings.values():
            values_length = len(values)
            if values_length == 0:
                continue

            if length is None:
                length = values_length
                continue

            if length != values_length:
                print('Register readings have different lengths', file=sys.stderr)
                print_clearly_note()
                sys.exit(1)

        if length is None:
            print('Report file does not contain any register readings', file=sys.stderr)
            sys.exit(1)

        if len(self.cpu_readings) > 0 and len(self.cpu_readings) != length:
            print('Length of CPU readings does not match length of register readings', file=sys.stderr)
            print_clearly_note()
            sys.exit(1)

        if len(self.gpu_readings) > 0 and len(self.gpu_readings) != length:
            print('Length of GPU readings does not match length of register readings', file=sys.stderr)
            print_clearly_note()
            sys.exit(1)

def make_graphs(data, opts):
    num = len(data.register_readings)
    width = math.ceil(math.sqrt(num))
    height = math.ceil(math.sqrt(num))

    fig, axs = plot.subplots(height, width)

    for i, (reg, values) in enumerate(data.register_readings.items()):
        row = i // width
        col = i % width

        ax_reg = axs[row, col]
        ax_reg.plot(values, color=opts.register_color, label='Register')
        ax_reg.set_title(reg)

        ax_cpu = None
        ax_gpu = None

        if data.cpu_readings:
            ax_cpu = ax_reg.twinx()
            ax_cpu.plot(data.cpu_readings, color=opts.cpu_color, label='CPU Temperature')

        if data.gpu_readings:
            ax_gpu = ax_reg.twinx()
            ax_gpu.plot(data.gpu_readings, color=opts.gpu_color, label='GPU Temperature')

        if ax_cpu is not None and ax_gpu is not None:
            ax_gpu.set_ylim(ax_cpu.get_ylim())

    fig.subplots_adjust(
        left=None, bottom=None, right=None, top=None, wspace=1, hspace=1
    )

    plot.show()

def read_dump(filename, decimal=False):
    if decimal:
        base = 10
    else:
        base = 16

    data = Data()

    with open(filename, "r") as fh:
        for line in fh:
            values = line.strip().split(",")
            first = values.pop(0)

            if first == '@CPU':
                data.cpu_readings = [float(v) for v in values]
            elif first == '@GPU':
                data.gpu_readings = [float(v) for v in values]
            else:
                data.register_readings[first] = [int(v, base) for v in values]

    return data

argp = argparse.ArgumentParser()
argp.add_argument('file')
argp.add_argument('-d', '--decimal', action='store_true')
argp.add_argument('--register-color', default='blue')
argp.add_argument('--cpu-color', default='red')
argp.add_argument('--gpu-color', default='magenta')
opts = argp.parse_args()

data = read_dump(opts.file, opts.decimal)
data.validate_or_die()
make_graphs(data, opts)
