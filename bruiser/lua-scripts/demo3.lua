
local demo3 = {}

function demo3.init()
  local wasm = require("wasm")
  local wasm_file = "../wasm/test/injected.wasm"
  local wasm_module = Wasm_Module()
  local table_type = table_type_t()
  local resizable_limit = resizable_limit_t()
  table_type:set_resizable_limit(resizable_limit)
end

function demo3.getmodule_py()
  local wasm = require("wasm")
  local wasm_module = Wasm_Module()
  local wasm_module = objload("dwasm", "elf_get_obj_names", elf_file, "symbol_list")
end

return demo3
