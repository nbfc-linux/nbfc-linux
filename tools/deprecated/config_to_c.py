#!/usr/bin/python3 -B

'''
DEPRECATED

This script was used to transform a XML file to C code
'''

import sys, json
from collections import OrderedDict
from config import *

class ArrayOf:
    def __init__(self, array, index, size):
        self.array, self.index, self.size = array, index, size

    def to_cpp(self):
        return '{ &%s[%d], %d }' % (self.array, self.index, self.size)

class EmptyArray:
    def to_cpp(self):
        return '{NULL, 0}'

class ArrayStorage(OrderedDict):
    def replace(self, obj):
        if len(obj) == 0:
            return EmptyArray()

        if obj.type not in self:
            self[obj.type] = obj
            return ArrayOf('array_'+obj.type, 0, len(obj))

        arr = self[obj.type]
        if len(arr) >= 1 and len(obj) == 1:
            #try: print(arr[0], obj[0], file=sys.stderr)
            #except:pass

            #try:
            #    if arr[0].to_cpp() == obj[0].to_cpp():
            #        print(arr[0] == obj[0], arr.index(obj[0]), file=sys.stderr)
            #        raise Exception("FUCK")
            #except ValueError:
            #    pass

            try:    return ArrayOf('array_'+obj.type, arr.index(obj[0]), 1)
            except: pass

        ref = ArrayOf('array_'+obj.type, len(arr), len(obj))
        for o in obj: 
            arr.append(o)
        return ref

    def to_cpp(self):
        return '\n'.join('%s array_%s[] = %s;' % (
            type, type, array.to_cpp()) for type, array in self.items())

arrays = ArrayStorage()

def visit(obj):
    if type(obj) is Struct:
        for name in list(obj.keys()):
            obj[name] = visit(obj[name])
        return obj
    elif type(obj) is Array:
        for i in range(len(obj)):
            obj[i] = visit(obj[i])
        return arrays.replace(obj)
    else:
        return obj

all_configs = Array()
all_configs.type = 'Config'
for f in sys.argv[1:]:
    try:
        all_configs.append(parse_xml_file(f, True))
    except Exception as e:
        print(f, e, file=sys.stderr)

all_configs = visit(all_configs)
#r = visit(r)

print(arrays.to_cpp()) #, '\n>>>>>', all_configs.to_cpp())

#print(json.dumps(r, default=lambda o: o.to_json(), indent=1))
#print(r.to_cpp())

