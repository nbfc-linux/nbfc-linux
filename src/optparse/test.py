#!/usr/bin/python3

import argparse

argp = argparse.ArgumentParser()

argp.add_argument("-F|--flag", action='store_true', dest='F')
argp.add_argument("-u|--unsigned", type=int, dest='u')
argp.add_argument("-i|--int", type=int, dest='i')
argp.add_argument("-f|--float", type=float, dest='f')
argp.add_argument("-d|--double", type=float, dest='d')
argp.add_argument("-D|--long-double", type=float, dest='D')
argp.add_argument("-c|--char", type=str, dest='c')
argp.add_argument("-b|--bool", type=bool, dest='b')
argp.add_argument("-s|--string", dest='s')
argp.add_argument("-o|--optional", nargs='?', dest='o')
argp.add_argument("-q|--quiet", action='store_true', dest='q')
argp.add_argument("--unhandeled", action='store_true')
argp.add_argument("-r|--repeat", type=int, dest='r')
argp.add_argument("command")

subg = argp.add_subparsers()
subp = subg.add_parser('positionals')
subp.add_argument('pos1')

#static const cli99_option cli_options[] = {
#define G1 cli99_exclusive_group(1)
#define G2 cli99_exclusive_group(2)
#  {"-A|--exclusive-A",         -'A', G1 |      cli99_no_argument},
#  {"-B|--exclusive-B",         -'B', G1 | G2 | cli99_no_argument},
#  {"-C|--exclusive-C",         -'C',      G2 | cli99_no_argument},
#undef G1
#undef G2

def parse_opts():
    opts = argp.parse_args()

    for opt in ('F', 'c', 'f', 'i', 'u', 'b', 's', 'o', 'command', 'pos1'):
        print('-'+opt, getattr(opts, opt) if hasattr(opts, opt) else '')


parse_opts()

