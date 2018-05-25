from enum import Enum

SectionID = {0:"custom", 1:"type", 2:"import", 3:"function", 4:"table", 5:"memory", 6:"global", 7:"export", 8:"start", 9:"element", 10:"code", 11:"data", 63:"unknown"}

class RelocType(Enum):
    R_WEBASSEMBLY_FUNCTION_INDEX_LEB = 0
    R_WEBASSEMBLY_TABLE_INDEX_SLEB = 1
    R_WEBASSEMBLY_TABLE_INDEX_I32 = 2
    R_WEBASSEMBLY_MEMORY_ADDR_LEB = 3
    R_WEBASSEMBLY_MEMORY_ADDR_SLEB = 4
    R_WEBASSEMBLY_MEMORY_ADDR_I32 = 5
    R_WEBASSEMBLY_TYPE_INDEX_LEB = 6
    R_WEBASSEMBLY_GLOBAL_INDEX_LEB = 7
    R_WEBASSEMPLY_FUNCTION_OFFSET_I32 = 8
    R_WEBASSEMBLY_SECTION_OFFSET_I32 = 9

class LinkingSubsection(Enum):
    WASM_SEGMENT_INFO = 5
    WASM_INIT_FUNCS = 6
    WASM_COMDAT_INFO = 7
    WASM_SYMBOL_TABLE = 8

class TypeType(Enum):
    none = 1
    lebu = 2
    lebs = 3
    flot = 4
    dobl = 5

class Syminfo_Kind():
    SYMTAB_FUNCTION = 0
    SYMTAB_DATA = 1
    SYMTAB_GLOBAL = 2
    SYMTAB_SECTION = 3

TypeKS = [['uint8', 8, TypeType.none], ['uint16', 16, TypeType.none],
          ['uint32', 32, TypeType.none], ['uint64', 64, TypeType.none],
          ['varuint1', 1, TypeType.lebu], ['varuint7', 7, TypeType.lebu],
          ['varuint32', 32, TypeType.lebu], ['varuint64', 64, TypeType.lebu],
          ['varint1', 1, TypeType.lebs], ['varint7', 7, TypeType.lebs],
          ['varint32', 32, TypeType.lebs], ['varint64', 64, TypeType.lebs]]

TypeDic = {'uint8': 1, 'uint16': 2, 'uint32': 4, 'uint64': 8,
           'varuint1': 1, 'varuint7': 1, 'varuint32': 4, 'varuint64': 8,
           'varint1': 1, 'varint7': 1, 'varint32': 4, 'varint64': 8}

# holds the version 1.0 wasm opcodes and immediates
class WASM_OP_Code:
    version_number = 0x01
    magic_number = 0x6d736100
    PAGE_SIZE = 65536
    uint8 = 1
    uint16 = 2
    uint32 = 4
    uint64 = 8
    varuint1 = 1
    varuint7 = 1
    varuint32 = 4
    varuint64 = 8
    varint1 = 1
    varint7 = 1
    varint32 = 4
    varint64 = 8
    floatt = 4
    doublet = 8

    all_ops = [('i32', '7f', False), ('i64', '7e', False), ('f32', '7d', False),
                ('f64', '7c', False), ('anyfunc', '7b', False),
                ('func', '60', False), ('empty_block_type', '40', False),
                ('unreachable', '00', False), ('nop', '01', False),
                ('block', '02', True, ('varuint7')),
                ('loop', '03', True, ('varuint7')),
                ('if', '04', True, ('varuint7')), ('else', '05', False),
                ('end', '0b', False), ('br', '0c', True, ('varuint32')),
                ('br_if', '0d', True, ('varuint32')),
                ('br_table', '0e', True, ('varuint32', 'varuint32', 'varuint32')),
                ('return', '0f', False), ('call', '10', True, ('varuint32')),
                ('call_indirect', '11', True, ('varuint32', 'varuint1')),
                ('drop', '1a', False), ('select', '1b', False),
                ('get_local', '20', True, ('varuint32')),
                ('set_local', '21', True, ('varuint32')),
                ('tee_local', '22', True, ('varuint32')),
                ('get_global', '23', True, ('varuint32')),
                ('set_global', '24', True, ('varuint32')),
                ('i32.load', '28', True, ('varuint32', 'varuint32')),
                ('i64.load', '29', True, ('varuint32', 'varuint32')),
                ('f32.load', '2a', True, ('varuint32', 'varuint32')),
                ('f64.load', '2b', True, ('varuint32', 'varuint32')),
                ('i32.load8_s', '2c', True, ('varuint32', 'varuint32')),
                ('i32.load8_u', '2d', True, ('varuint32', 'varuint32')),
                ('i32.load16_s', '2e', True, ('varuint32', 'varuint32')),
                ('i32.load16_u', '2f', True, ('varuint32', 'varuint32')),
                ('i64.load8_s', '30', True, ('varuint32', 'varuint32')),
                ('i64.load8_u', '31', True, ('varuint32', 'varuint32')),
                ('i64.load16_s', '32', True, ('varuint32', 'varuint32')),
                ('i64.load16_u', '33', True, ('varuint32', 'varuint32')),
                ('i64.load32_s', '34', True, ('varuint32', 'varuint32')),
                ('i64.load32_u', '35', True, ('varuint32', 'varuint32')),
                ('i32.store', '36', True, ('varuint32', 'varuint32')),
                ('i64.store', '37', True, ('varuint32', 'varuint32')),
                ('f32.store', '38', True, ('varuint32', 'varuint32')),
                ('f64.store', '39', True, ('varuint32', 'varuint32')),
                ('i32.store8', '3a', True, ('varuint32', 'varuint32')),
                ('i32.store16', '3b', True, ('varuint32', 'varuint32')),
                ('i64.store8', '3c', True, ('varuint32', 'varuint32')),
                ('i64.store16', '3d', True, ('varuint32', 'varuint32')),
                ('i64.store32', '3e', True, ('varuint32', 'varuint32')),
                ('current_memory', '3f', True, ('varuint1')),
                ('grow_memory', '40', True, ('varuint1')),
                ('i32.const', '41', True, ('varint32')),
                ('i64.const', '42', True, ('varint64')),
                ('f32.const', '43', True, ('uint32')),
                ('f64.const', '44', True, ('uint64')),
                ('i32.eqz', '45', False), ('i32.eq', '46', False),
                ('i32.ne', '47', False), ('i32.lt_s', '48', False),
                ('i32.lt_u', '49', False), ('i32.gt_s', '4a', False),
                ('i32.gt_u', '4b', False), ('i32.le_s', '4c', False),
                ('i32.le_u', '4d', False), ('i32.ge_s', '4e', False),
                ('i32.ge_u', '4f', False), ('i64.eqz', '50', False),
                ('i64.eq', '51', False), ('i64.ne', '52', False),
                ('i64.lt_s', '53', False), ('i64.lt_u', '54', False),
                ('i64.gt_s', '55', False), ('i64.gt_u', '56', False),
                ('i64.le_s', '57', False), ('i64.le_u', '58', False),
                ('i64.ge_s', '59', False), ('i64.ge_u', '5a', False),
                ('f32.eq', '5b', False), ('f32.ne', '5c', False),
                ('f32.lt', '5d', False), ('f32.gt', '5e', False),
                ('f32.le', '5f', False), ('f32.ge', '60', False),
                ('f64.eq', '61', False), ('f64.ne', '62', False),
                ('f64.lt', '63', False), ('f64.gt', '64', False),
                ('f64.le', '65', False), ('f64.ge', '66', False),
                ('i32.clz', '67', False), ('i32.ctz', '68', False),
                ('i32.popcnt', '69', False), ('i32.add', '6a', False),
                ('i32.sub', '6b', False), ('i32.mul', '6c', False),
                ('i32.div_s', '6d', False), ('i32.div_u', '6e', False),
                ('i32.rem_s', '6f', False), ('i32.rem_u', '70', False),
                ('i32.and', '71', False), ('i32.or', '72', False),
                ('i32.xor', '73', False), ('i32.shl', '74', False),
                ('i32.shr_s', '75', False), ('i32.shr_u', '76', False),
                ('i32.rotl', '77', False), ('i32.rotr', '78', False),
                ('i64.clz', '79', False), ('i64.ctz', '7a', False),
                ('i64.popcnt', '7b', False), ('i64.add', '7c', False),
                ('i64.sub', '7d', False), ('i64.mul', '7e', False),
                ('i64.div_s', '7f', False), ('i64.div_u', '80', False),
                ('i64.rem_s', '81', False), ('i64.rem_u', '82', False),
                ('i64.and', '83', False), ('i64.or', '84', False),
                ('i64.xor', '85', False), ('i64.shl', '86', False),
                ('i64.shr_s', '87', False), ('i64.shr_u', '88', False),
                ('i64.rotl', '89', False), ('i63.rotr', '8a', False),
                ('f32.abs', '8b', False), ('f32.neg', '8c', False),
                ('f32.ceil', '8d', False),  ('f32.floor', '8e', False),
                ('f32.trunc', '8f', False), ('f32.nearest', '90', False),
                ('f32.sqrt', '91', False), ('f32.add', '92', False),
                ('f32.sub', '93', False), ('f32.mul', '94', False),
                ('f32.div', '95', False), ('f32.min', '96', False),
                ('f32.max', '97', False), ('f32.copysign', '98', False),
                ('f64.abs', '99', False), ('f64.neg', '9a', False),
                ('f64.ceil', '9b', False), ('f64.floor', '9c', False),
                ('f64.trunc', '9d', False), ('f64.nearest', '9e', False),
                ('f64.sqrt', '9f', False), ('f64.add', 'a0', False),
                ('f64.sub', 'a1', False), ('f64.mul', 'a2', False),
                ('f64.div', 'a3', False), ('f64.min', 'a4', False),
                ('f64.max', 'a5', False), ('f64.copysign', 'a6', False),
                ('i32.wrap/i64', 'a7', False), ('i32.trunc_s/f32', 'a8', False),
                ('i32.trunc_u/f32', 'a9', False),
                ('i32.trunc_s/f64', 'aa', False),
                ('i32.trunc_u/f64', 'ab', False),
                ('i64.extend_s/i32', 'ac', False),
                ('i64.extend_u/i32', 'ad', False),
                ('i64.trunc_s/f32', 'ae', False),
                ('i64.trunc_u/f32', 'af', False),
                ('i64.trunc_s/f64', 'b0', False),
                ('i64.trunc_u/f64', 'b1', False),
                ('f32.convert_s/i32', 'b2', False),
                ('f32.convert_u/i32', 'b3', False),
                ('f32.convert_s/i64', 'b4', False),
                ('f32.convert_u/i64', 'b5', False),
                ('f32.demote/f64', 'b6', False),
                ('f64.convert_s/i32', 'b7', False),
                ('f64.convert_u/i32', 'b8', False),
                ('f64.convert_s/i64', 'b9', False),
                ('f64.convert_u/i64', 'ba', False),
                ('f64.promote/f32', 'bb', False),
                ('i32.reinterpret/f32', 'bc', False),
                ('i64.reinterpret/f64', 'bd', False),
                ('f32.reinterpret/i32', 'be', False),
                ('f64.reinterpret/i64', 'bf', False)]

    type_ops = [('i32', '7f'), ('i64', '7e'), ('f32', '7d'),
                ('f64', '7c'), ('anyfunc', '7b'), ('func', '60'),
                ('empty_block_type', '40')]
    type_ops_dict = dict(type_ops)
    type_ops_dict_rev = {v: k for k, v in type_ops_dict.items()}

    control_flow_ops = [('unreachable', '00'), ('nop', '01'),
                        ('block', '02'), ('loop', '03'),
                        ('if', '04'), ('else', '05'),
                        ('end', '0b'), ('br', '0c'),
                        ('br_if', '0d'), ('br_table', '0e'),
                        ('return', '0f')]
    control_flow_ops_dict = dict(control_flow_ops)
    control_flow_ops_dict_rev = {v: k for k, v in control_flow_ops_dict.items()}

    call_ops = [('call', '10'), ('call_indirect', '11')]
    call_ops_dict = dict(call_ops)
    call_ops_dict_rev = {v: k for k, v in call_ops_dict.items()}

    param_ops = [('drop', '1a'), ('select', '1b')]
    param_ops_dict = dict(param_ops)
    param_ops_dict_rev = {v: k for k, v in param_ops_dict.items()}

    var_access = [('get_local', '20'), ('set_local', '21'),
                    ('tee_local', '22'), ('get_global', '23'),
                    ('set_global', '24')]
    var_access_dict = dict(var_access)
    var_access_dict_rev = {v: k for k, v in var_access_dict.items()}

    mem_ops = [('i32.load', '28'), ('i64.load', '29'),
                ('f32.load', '2a'), ('f64.load', '2b'),
                ('i32.load8_s', '2c'), ('i32.load8_u', '2d'),
                ('i32.load16_s', '2e'),  ('i32.load16_u', '2f'),
                ('i64.load8_s', '30'), ('i64.load8_u', '31'),
                ('i64.load16_s', '32'), ('i64.load16_u', '33'),
                ('i64.load32_s', '34'), ('i64.load32_u', '35'),
                ('i32.store', '36'), ('i64.store', '37'),
                ('f32.store', '38'), ('f64.store', '39'),
                ('i32.store8', '3a'), ('i32.store16', '3b'),
                ('i64.store8', '3c'), ('i64.store16', '3d'),
                ('i64.store32', '3e'), ('current_memory', '3f'),
                ('grow_memory', '40')]
    mem_ops_dict = dict(mem_ops)
    mem_ops_dict_rev = {v: k for k, v in mem_ops_dict.items()}

    consts = [('i32.const', '41'), ('i64.const', '42'),
              ('f32.const', '43'), ('f64', '44')]
    consts_dict = dict(consts)
    consts_dict_rev = {v: k for k, v in consts_dict.items()}

    comp_ops = [('i32.eqz', '45'), ('i32.eq', '46'), ('i32.ne', '47'),
                ('i32.lt_s', '48'), ('i32.lt_u', '49'),
                ('i32.gt_s', '4a'), ('i32.gt_u', '4b'),
                ('i32.le_s', '4c'), ('i32.le_u', '4d'),
                ('i32.ge_s', '4e'), ('i32.ge_u', '4f'),
                ('i64.eqz', '50'), ('i64.eq', '51'),
                ('i64.ne', '52'), ('i64.lt_s', '53'),
                ('i64.lt_u', '54'), ('i64.gt_s', '55'),
                ('i64.gt_u', '56'), ('i64.le_s', '57'),
                ('i64.le_u', '58'), ('i64.ge_s', '59'),
                ('i64.ge_u', '5a'), ('f32.eq', '5b'),
                ('f32.ne', '5c'), ('f32.lt', '5d'),
                ('f32.gt', '5e'), ('f32.le', '5f'),
                ('f32.ge', '60'), ('f64.eq', '61'),
                ('f64.ne', '62'), ('f64.lt', '63'),
                ('f64.gt', '64'), ('f64.le', '65'),
                ('f64.ge', '66')]
    comp_ops_dict = dict(comp_ops)
    comp_ops_dict_rev = {v: k for k, v in comp_ops_dict.items()}

    num_ops = [('i32.clz', '67'), ('i32.ctz', '68'),
               ('i32.popcnt', '69'), ('i32.add', '6a'),
               ('i32.sub', '6b'), ('i32.mul', '6c'),
               ('i32.div_s', '6d'), ('i32.div_u', '6e'),
               ('i32.rem_s', '6e'), ('i32.rem_u', '70'),
               ('i32.and', '71'), ('i32.or', '72'),
               ('i32.xor', '73'), ('i32.shl', '74'),
               ('i32.shr_s', '75'), ('i32.shr_u', '76'),
               ('i32.rotl', '77'), ('i32.rotr', '78'),
               ('i64.clz', '79'), ('i64.ctz', '7a'),
               ('i64.popcnt', '7b'), ('i64.add', '7c'),
               ('i64.sub', '7d'), ('i64.mul', '7e'),
               ('i64.div_s', '7f'), ('i64.div_u', '80'),
               ('i64.rem_s', '81'), ('i64.rem_u', '82'),
               ('i64.and', '83'), ('i64.or', '84'),
               ('i64.xor', '85'), ('i64.shl', '86'),
               ('i64.shr_s', '87'), ('i64.shr_u', '88'),
               ('i64.rotl', '89'), ('i63.rotr', '8a'),
               ('f32.abs', '8b'), ('f32.neg', '8c'),
               ('f32.ceil', '8d'),  ('f32.floor', '8e'),
               ('f32.trunc', '8f'), ('f32.nearest', '90'),
               ('f32.sqrt', '91'), ('f32.add', '92'),
               ('f32.sub', '93'), ('f32.mul', '94'),
               ('f32.div', '95'), ('f32.min', '96'),
               ('f32.max', '97'), ('f32.copysign', '98'),
               ('f64.abs', '99'), ('f64.neg', '9a'),
               ('f64.ceil', '9b'), ('f64.floor', '9c'),
               ('f64.trunc', '9d'), ('f64.nearest', '9e'),
               ('f64.sqrt', '9f'), ('f64.add', 'a0'),
               ('f64.sub', 'a1'), ('f64.mul', 'a2'),
               ('f64.div', 'a3'), ('f64.min', 'a4'),
               ('f64.max', 'a5'), ('f64.copysign', 'a6')]
    num_ops_dict = dict(num_ops)
    num_ops_dict_rev = {v: k for k, v in num_ops_dict.items()}

    conversion = [('i32.wrap/i64', 'a7'),
                    ('i32.trunc_s/f32', 'a8'),
                    ('i32.trunc_u/f32', 'a9'),
                    ('i32.trunc_s/f64', 'aa'),
                    ('i32.trunc_u/f64', 'ab'),
                    ('i64.extend_s/i32', 'ac'),
                    ('i64.extend_u/i32', 'ad'),
                    ('i64.trunc_s/f32', 'ae'),
                    ('i64.trunc_u/f32', 'af'),
                    ('i64.trunc_s/f64', 'b0'),
                    ('i64.trunc_u/f64', 'b1'),
                    ('f32.convert_s/i32', 'b2'),
                    ('f32.convert_u/i32', 'b3'),
                    ('f32.convert_s/i64', 'b4'),
                    ('f32.convert_u/i64', 'b5'),
                    ('f32.demote/f64', 'b6'),
                    ('f64.convert_s/i32', 'b7'),
                    ('f64.convert_u/i32', 'b8'),
                    ('f64.convert_s/i64', 'b9'),
                    ('f64.convert_u/i64', 'ba'),
                    ('f64.promote/f32', 'bb')]
    conversion_dict = dict(conversion)
    conversion_dict_rev = {v: k for k, v in conversion_dict.items()}

    reinterpretations = [('i32.reinterpret/f32', 'bc'),
                         ('i64.reinterpret/f64', 'bd'),
                         ('f32.reinterpret/i32', 'be'),
                         ('f64.reinterpret/i64', 'bf')]
    reinterpretations_dict = dict(reinterpretations)
    reinterpretations_dict_rev = {v: k for k,
                                  v in reinterpretations_dict.items()}

    section_code = [('type', '01'), ('import', '02'),
                    ('function', '03'), ('table', '04'),
                    ('memory', '05'), ('global', '06'),
                    ('export', '07'), ('start', '08'),
                    ('element', '09'), ('code', '0a'),
                    ('data', '0b'), ('custom', '00')]
    section_code_dict = dict(section_code)
    section_code_dict_rev = {v: k for k, v in section_code_dict.items()}
