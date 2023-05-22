#!/usr/bin/python3 -B
import sys, os, json
from collections import OrderedDict

def find_config_json():
    return os.path.join(os.path.dirname(sys.argv[0]), 'config.json')

# =============================================================================
# Config definition classes
# =============================================================================

class StructDefinition:
    def __init__(self, name, fields, help=""):
        self.name   = name
        self.type   = 'struct'
        self.fields = fields
        self.help   = help

    def __iter__(self):
        return iter(self.fields)

    def __getitem__(self, k):
        try:
            return next(filter(lambda f: f.name == k, self.fields))
        except StopIteration:
            raise Exception("%s: Key not found: %s" % (self.name, k))

    def __repr__(self):
        return f'''Struct({repr(self.name)}, {self.fields})'''

class FieldDefinition:
    def __init__(self, name_, type_, **optional):
        type           = type_.replace('const char*', 'str')
        type           = type.replace('(', '_')
        type           = type.replace(')', '_')
        type           = type.replace(' ', '_')
        type           = type.strip('_')
        self.name      = name_
        self.type      = type_
        self.help      = optional.get('help', "")
        self.valid     = optional.get('valid', None)
        self.unset     = optional.get('unset', '%s_Unset' % type)
        self.default   = optional.get('default', None)
        self.is_unset  = optional.get('is_unset', None)
        self.from_json = optional.get('from_json', '%s_FromJson' % type)

    def __repr__(self):
        return f'''Field({repr(self.name)}, {self.type}, help = {repr(self.help)})'''

# =============================================================================
#                  Those types reflect the types used in C
# =============================================================================

# =============================================================================
# Primitives
# =============================================================================

class CBool:
    @staticmethod
    def parse(s):        return CBool({'FALSE':False,'TRUE':True}[s.upper()])
    def to_cpp(self):    return ('Boolean_False','Boolean_True')[self.v]
    def to_json(self):   return self.v
    def __init__(self, v): self.v = v
    def __eq__(self, o): return self.v == o.v

class CInt(int):
    @staticmethod
    def parse(s):     return CInt(int(s))
    def to_cpp(self): return "%d" % self

class CFloat(float):
    @staticmethod
    def parse(s):        return CFloat(float(s))
    def to_cpp(self):    return "%.2f" % self
    def __eq__(self, o): return abs(self - o) <= 0.06

class CString(str):
    @staticmethod
    def parse(s):     return CString(s)
    def to_cpp(self): return "\"%s\"" % self

class COverrideTargetOperation(str):
    @staticmethod
    def parse(s):     return COverrideTargetOperation(s)
    def to_cpp(self): return "OverrideTargetOperation_"+self

class CRegisterWriteOccasion(str):
    @staticmethod
    def parse(s):     return CRegisterWriteOccasion(s)
    def to_cpp(self): return "RegisterWriteOccasion_"+self

class CRegisterWriteMode(str):
    @staticmethod
    def parse(s):     return CRegisterWriteMode(s)
    def to_cpp(self): return "RegisterWriteMode_"+self

# =============================================================================
# Complex
# =============================================================================

class Struct(OrderedDict):
    def to_cpp(self):
        return '{%s}'% ',\n'.join(f.to_cpp() for f in self.values())

class Array(list):
    def to_cpp(self):
        return '{%s}'% ',\n'.join(f.to_cpp() for f in self)

with open(find_config_json(), 'r') as fh:
    definitions = json.load(fh)
    structs = OrderedDict()

    for struct in definitions:
        fields = []
        for field in struct['fields']:
            fields.append(FieldDefinition(field['name'], field['type'], **field))
        structs[struct['name']] = StructDefinition(struct['name'], fields, struct.get('help', ''))

    #for struct_name, fields in structs.items():
    #    new_fields = []
    #    _help = fields.pop('_help', None)
    #    for field_name, field in fields.items():
    #        new_fields.append(FieldDefinition(field_name, field['type'], **field))
    #    structs[struct_name] = StructDefinition(struct_name, new_fields, help=_help)

# =============================================================================
# XML Parsing
# =============================================================================

def xml_remove_comments(node):
    from lxml import etree
    for n in node:
        if isinstance(n, (etree._Comment,)):
            node.remove(n)
        xml_remove_comments(n)

def handle_xml_node(node, definition):
    if node.tag == 'FanControlConfigV2':
        pass # this is a exception
    elif node.tag != definition.name:
        raise Exception('XML node "%s" != "%s"' % (node.tag, definition.name))

    if definition.type == 'struct':
        ret = Struct()

        for n in node:
            ret[n.tag] = handle_xml_node(n, definition[n.tag])

    elif definition.type.startswith('array_of('):
        definition = structs[definition.type.split('(')[1].strip(')')]
        ret = Array()

        for n in node:
            ret.append(handle_xml_node(n, definition))

    else:
        ret = {
            'const char*':             CString,
            'int':                     CInt,
            'short':                   CInt,
            'float':                   CFloat,
            'Boolean':                 CBool,
            'OverrideTargetOperation': COverrideTargetOperation,
            'RegisterWriteMode':       CRegisterWriteMode,
            'RegisterWriteOccasion':   CRegisterWriteOccasion
        }[definition.type].parse(node.text)

    return ret


def parse_xml_file(file):
    from lxml import etree
    tree = etree.parse(file)
    root = tree.getroot()
    xml_remove_comments(root)
    return handle_xml_node(root, structs['ModelConfig'])


def write_header(fh):
    p = lambda *a,**kw: print(*a, **kw, file=fh)

    p('/* Auto generated code %r */\n' % sys.argv);
    for name, struct in structs.items():
        p(f'struct {name} {{')
        for field in struct:
            p(f'\t{field.type:<15} {field.name};')
        p('};')
        p('')
        p(f'typedef struct {name} {name};')
        p(f'declare_array_of({name});')
        p(f'Error* {struct.name}_FromJson({struct.name}*, const nx_json*);')
        p(f'Error* {struct.name}_ValidateFields({struct.name}*);')
        p('')

def write_source(fh):
    p = lambda *a,**kw: print(*a, **kw, file=fh)

    p('/* Auto generated code %r */\n' % sys.argv);
    for struct in structs.values():
        p(f'struct {struct.name} {struct.name}_Unset = {{')
        for field in struct:
            p(f'\t{field.unset},')
        p('};\n')

        write_validate_fields(struct, fh)
        write_parse_struct(struct, fh)
        p('')

def write_validate_fields(struct, fh):
    p = lambda *a,**kw: print(*a, **kw, file=fh)

    p(f'Error* {struct.name}_ValidateFields({struct.name}* self) {{', end='')
    for field in struct:
        if field.is_unset is not None:
            is_unset = field.is_unset.replace('parameter', f'self->{field.name}')
        else:
            is_unset = f'self->{field.name} == {field.unset}'

        if field.default is not None:
            set_or_throw = f'self->{field.name} = {field.default}'
        else:
            set_or_throw = f'return err_string(0, "{field.name}: Missing option")'

        if field.valid is not None:
            is_valid = field.valid.replace('parameter', f'self->{field.name}')

        p()
        p(f'\tif ({is_unset})')
        p(f'\t\t{set_or_throw};')
        if field.valid is not None:
            p(f'\telse if (! ({is_valid}))')
            p(f'\t\treturn err_string(0, "{field.name}: requires: {field.valid}");')

    p('\treturn err_success();')
    p('}\n')

def write_parse_struct(struct, fh):
    p = lambda *a,**kw: print(*a, **kw, file=fh)

    p(f'Error* {struct.name}_FromJson({struct.name}* obj, const nx_json* json) {{')
    p( '\tError* e = NULL;')
    p(f'\t*obj = {struct.name}_Unset;')
    p('')
    p( '\tif (!json || json->type != NX_JSON_OBJECT)')
    p( '\t\treturn err_string(0, "not a JSON object");')
    p( '')
    p( '\tnx_json_for_each(c, json) {')
    p( '\t\tif (0);')

    for field in struct:
        p(f'\t\telse if (!strcmp(c->key, "{field.name}"))')
        p(f'\t\t\te = {field.from_json}(&obj->{field.name}, c);')

    p( '\t\telse')
    p( '\t\t\te = err_string(0, "Unknown option");')
    p( '\t\tif (e) return err_string(e, c->key);')
    p( '\t}')
    p( '\treturn err_success();')
    p( '}')


if __name__ == '__main__':
    if   sys.argv[1] == 'header': write_header(sys.stdout)
    elif sys.argv[1] == 'source': write_source(sys.stdout)
    else:
        print('Usage:', sys.argv[0], 'header|source', file=sys.stderr)
        sys.exit(1)

