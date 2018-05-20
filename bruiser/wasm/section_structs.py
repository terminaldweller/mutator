# contains the data classes we use to hold the information of a module
class WASM_SECTION(object):
    def __init__(self):
        self.section_id = int()
        self.string = str()
        self.payload_length = int()
        self.is_custom_section = bool()
        self.name_len = int()
        self.name = str()
        self.payload_data = bytes()


class Reloc_Entry_1():
    def __init__(self):
        self.offset = int()
        self.index = int()


class Reloc_Entry_2():
    def __init__(self):
        self.offset = int()
        self.index = int()
        self.addend = int()


class Relocation_Section():
    def __int__(self):
        self.section_id = int()
        self.name_length = int()
        self.name = str()
        self.count = int()
        self.entries = int()


class Func_Type():
    def __init__(self):
        self.form = int()
        self.param_cnt = int()
        self.param_types = []
        self.return_cnt = int()
        self.return_type = []


class Global_Type():
    def __init__(self):
        self.content_type = int()
        self.mutability = int()


class Resizable_Limits():
    def __init__(self):
        self.flags = int()
        self.initial = int()
        self.maximum = int()


class Table_Type():
    def __init__(self):
        self.element_type = int()
        self.limit = Resizable_Limits()


class External_Kind():
    def __init__(self):
        self.Function = 0
        self.Table = 1
        self.Memory = 2
        self.Global = 3


class Memory_Type():
    def __init__(self):
        self.limits = [Resizable_Limits()]


# @DEVI-FIXME-
class Init_Expr():
    pass


class Type_Section():
    def __init__(self):
        self.count = []
        self.func_types = []


class Import_Entry():
    def __init__(self):
        self.module_len = int()
        self.module_str = []
        self.field_len = int()
        self.field_str = []
        self.kind = int()
        self.type = int()


class Import_Section():
    def __init__(self):
        self.count = []
        self.import_entry = []


class Function_Section():
    def __init__(self):
        self.count = []
        self.type_section_index = [int()]


class Table_Section():
    def __init__(self):
        self.count = []
        self.table_types = []


class Memory_Section():
    def __init__(self):
        self.count = []
        # Resizable_Limits
        self.memory_types = []


class Global_Variable():
    def __init__(self):
        self.global_type = Global_Type()
        self.init_expr = []


class Global_Section():
    def __init__(self):
        self.count = []
        # Global_Variable
        self.global_variables = []


class Export_Entry():
    def __init__(self):
        self.field_len = int()
        self.field_str = []
        self.kind = int()
        self.index = int()


class Export_Section():
    def __init__(self):
        self.count = []
        # Export_Entry
        self.export_entries = []


class Start_Section():
    def __init__(self):
        self.function_section_index = []


class Elem_Segment():
    def __init__(self):
        self.index = int()
        self.offset = []
        self.num_elem = int()
        self.elems = []


class Element_Section():
    def __init__(self):
        self.count = []
        # Elem_Segment
        self.elem_segments = []


class Local_Entry():
    def __init__(self):
        self.count = int()
        self.type = int()


class WASM_Ins():
    def __init__(self):
        self.opcode = str()
        self.opcodeint = int()
        self.operands = []


class Func_Body():
    def __init__(self):
        self.body_size = int()
        self.local_count = int()
        # Local_Entry
        self.locals = []
        # WASM_Ins
        self.code = []
        self.end = int()


class Code_Section():
    def __init__(self):
        self.count = []
        # Func_Body
        self.func_bodies = []


class Data_Segment():
    def __init__(self):
        self.index = int()
        self.offset = []
        self.size = int()
        self.data = []


class Data_Section():
    def __init__(self):
        self.count = []
        # Data_Segment
        self.data_segments = []


class Name_Type():
    Module = 0
    Function = 1
    Local = 2


class Name_Section_Entry(object):
    def __init__(self, name_type, name_payload_len, name_payload_data):
        self.name_type = name_type
        self.name_payload_len = name_payload_len
        self.name_payload_data = name_payload_data


class Name_Section(object):
    def __init__(self, name_section_entry):
        self.name_section_entry = []
        self.name_section_entry = name_section_entry


class Module_Name(object):
    def __init__(self, name_len, name_str):
        self.name_len = name_len
        self.name_str = name_str


class Naming(object):
    def __init__(self, index, name_len, name_str):
        self.index = index
        self.name_len = name_len
        self.name_str = name_str


class Name_Map(object):
    def __init__(self, count, naming):
        self.count = count
        self.naming = []
        self.naming = naming


# the module class
class Module():
    def __init__(self, type_section, import_section, function_section,
                 table_section, memory_section, global_section, export_section,
                 start_section, element_section, code_section, data_section):
        self.type_section = type_section
        self.import_section = import_section
        self.function_section = function_section
        self.table_section = table_section
        self.memory_section = memory_section
        self.global_section = global_section
        self.export_section = export_section
        self.start_section = start_section
        self.element_section = element_section
        self.code_section = code_section
        self.data_section = data_section

'''
class RT_INS_CELL(object):
    def __init__(self):
        label : str
        mnemonic : str
        ops : list
'''
