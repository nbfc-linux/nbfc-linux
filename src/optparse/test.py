#!/usr/bin/python3

import sys, subprocess

def test(cmdline, expected):
    p = subprocess.Popen(['./a.out'] + cmdline.split(), stdout=subprocess.PIPE)
    out = p.communicate()[0].decode('UTF-8')
    out = out.rstrip()
    if out != expected:
        print("Failed test:", cmdline)
        print("Got  %r" % out)
        print("Have %r" % expected)
        sys.exit(1)

t = test
t('test -F',            'command=test -F=True')
t('test -FF',           'command=test -F=True -F=True')
t('test -i 2',          'command=test -i=2')
t('test -i2',           'command=test -i=2')
t('test -FF',           'command=test -F=True -F=True')
t('test -Fi 2',         'command=test -F=True -i=2')
t('test -Fi2',          'command=test -F=True -i=2')
t('test -s foo',        'command=test -s=foo')
t('test --string foo',  'command=test -s=foo')
t('test --string=foo',  'command=test -s=foo')
#t('test -AB',           '')
