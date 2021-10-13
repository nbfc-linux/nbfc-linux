#!/usr/bin/python3 -B

import sys, os, json, config, argparse

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
        s = json.dumps(r, default=lambda o: o.to_json(), indent=1)

        # reconfigure temperature thresholds to account for the new logic:
        p = json.loads(s)
        thresholds = p['FanConfigurations'][0].get('TemperatureThresholds')
        if thresholds:
            thresholds.sort(key=lambda x: x['UpThreshold'])

            for i in range(len(thresholds)-1):
                thresholds[i]['UpThreshold'] = thresholds[i+1]['UpThreshold']
            thresholds[-1]['UpThreshold'] = p['CriticalTemperature']

            #if thresholds[0]['UpThreshold'] == 0:
            #    raise Exception('UpThreshold of 0 detected.')

            p['FanConfigurations'][0]['TemperatureThresholds'] = thresholds
            s = json.dumps(p, indent=1)

        s = s.replace('\n  ', '\n\t')

        json_file = os.path.join(opts.out_dir, basename[0:-4] + '.json')

        #print(infile, '->', json_file, file=sys.stderr)

        with open(json_file, 'w') as fh:
            fh.write(s)

    except Exception as e:
        print('Error:', infile, e, file=sys.stderr)


