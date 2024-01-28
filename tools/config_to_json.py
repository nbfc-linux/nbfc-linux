#!/usr/bin/python3 -B

import sys, os, json, config, argparse
from collections import OrderedDict

def OrderedDict_insert_at_top(dict, key, value):
    result = OrderedDict()
    result[key] = value
    for key, value in dict.items():
        result[key] = value
    return result

argp = argparse.ArgumentParser()
argp.add_argument('--out-dir', default='.')
argp.add_argument('infile', metavar='INPUT FILE', nargs='+')
opts = argp.parse_args()

for infile in opts.infile:
    try:
        _, basename = os.path.split(infile)

        if basename.lower().endswith('.json'):
            continue

        if not basename.lower().endswith('.xml'):
            raise Exception('Not an xml file')

        r = config.parse_xml_file(infile)
        r = OrderedDict_insert_at_top(r, 'LegacyTemperatureThresholdsBehaviour', True)
        s = json.dumps(r, default=lambda o: o.to_json(), indent=1)
        s = s.replace('\n  ', '\n\t')

        json_file = os.path.join(opts.out_dir, basename[0:-4] + '.json')

        #print(infile, '->', json_file, file=sys.stderr)

        with open(json_file, 'w') as fh:
            fh.write(s)

    except Exception as e:
        print('Error:', infile, e, file=sys.stderr)


