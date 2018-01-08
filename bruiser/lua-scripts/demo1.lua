--
-- get the .so object names
-- objload("elf_get_obj_names", "../bfd/test/test.so", "symbol_list")
--
-- get the .so object sizes
-- objload("elf_get_obj_sizes", "../bfd/test/test.so", "symbol_list")
--
-- get the .so function names
-- objload("elf_get_func_names", "../bfd/test/test.so", "symbol_list")
--
-- get the .so function code
-- objload("elf_get_func_code", "../bfd/test/test.so", "code_list")
--

function printObjNames()
  local c = objload("elf_get_obj_names", "../bfd/test/test.so", "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function printObjSizes()
  local c = objload("elf_get_obj_sizes", "../bfd/test/test.so", "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function printFuncNames()
  local c = objload("elf_get_func_names", "../bfd/test/test.so", "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function printFuncCode()
  local c = objload("elf_get_func_code", "../bfd/test/test.so", "code_list")
  for k,v in ipairs(c) do
    print(k,v)
    if #v ~= 0 then
      for k1,v1 in ipairs(v) do
        print(k1, v1)
      end
    end
  end
end

printObjNames()
printObjSizes()
printFuncNames()
printFuncCode()
