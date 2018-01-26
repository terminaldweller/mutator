--
-- get the .so object names in a table
-- objload("elf_get_obj_names", "../bfd/test/test.so", "symbol_list")
--
-- get the .so object sizes in a table
-- objload("elf_get_obj_sizes", "../bfd/test/test.so", "symbol_list")
--
-- get the .so function names in a table
-- objload("elf_get_func_names", "../bfd/test/test.so", "symbol_list")
--
-- get the .so function code in a table of tables
-- objload("elf_get_func_code", "../bfd/test/test.so", "code_list")
--
--------------------------------------------------------------------------------------------------------------
elf_file = "../bfd/test/test.so"
--elf_file = "../bfd/test/test"

function printObjNames()
  local c = objload("elf_get_obj_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function printObjSizes()
  local c = objload("elf_get_obj_sizes", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function printFuncNames()
  local c = objload("elf_get_func_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function printFuncCode()
  local c = objload("elf_get_func_code", elf_file, "code_list")
  for k,v in ipairs(c) do
    print(k,v)
    if #v ~= 0 then
      for k1,v1 in ipairs(v) do
        io.write(string.format('%02x', v1), " ")
      end
      io.write("\n")
    end
  end
end

function findMain()
  local c = objload("elf_get_func_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    if v == "'main'" then 
      io.write("main index is".." "..k.."\n")
      return k
    end
  end
end

function codeTables()
  local return_table = {}
  local func_name_table = objload("elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("elf_get_func_code", elf_file, "code_list")
  for i=1,#func_name_table,1 do
    return_table[func_name_table[i]] = code_table[i]
  end
  return return_table
end

function codeTableByName(name)
  local return_table = {}
  local func_name_table = objload("elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("elf_get_func_code", elf_file, "code_list")
  for k,v in ipairs(func_name_table) do
    if v == name then
      for k1, v1 in ipairs(code_table[k]) do
        table.insert(return_table, string.format('%02x', v1))
      end
      return return_table
    end
  end
  return nil
end

function codeTableByName_number(name)
  local return_table = {}
  local func_name_table = objload("elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("elf_get_func_code", elf_file, "code_list")
  for k,v in ipairs(func_name_table) do
    if v == name then
      for k1, v1 in ipairs(code_table[k]) do
        table.insert(return_table, v1)
      end
      return return_table
    end
  end
  return nil
end

function printFuncSizes()
  local func_name_table = objload("elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("elf_get_func_code", elf_file, "code_list")
  local counter = 1
  print("function sizes:")
  for k, v in ipairs(code_table) do
    print("code size for "..func_name_table[counter].." is".." "..#v)
    counter = counter + 1
  end
end

-- start of @placeholder
function xobjRegister(code_table, registration_name)
  -- lightuserdata
  local Xobjpointer
  return Xobjpointer
end
-- should be varargs
-- registration_name()
-- end of @placeholder

-- start of @placeholder
-- C--args = argc, arg_string, ffi_type, xptr, the_actual_args
-- lua--args = number, table, string, xptr, args
-- end of @placeholder
function callX(n_argc, t_string, str, index, t_values)
end

function main()
  printObjNames()
  printObjSizes()
  printFuncNames()
  printFuncCode()
  findMain()

  local code_table = codeTables()
  print(code_table["'main'"])
  for k,v in ipairs(code_table["'main'"]) do
    io.write(string.format('%02x', v), " ")
  end
  io.write("\n")

  local C_main_code = codeTableByName("'main'")
  for k, v in ipairs(C_main_code) do
    io.write(v, " ")
  end
  io.write("\n")

  local add2_code = codeTableByName_number("'add2'")
  local sub2_code = codeTableByName_number("'sub2'")
  local adddouble_code = codeTableByName_number("'adddouble'")
  local subdouble_code = codeTableByName_number("'subdouble'")
  local triple_code = codeTableByName_number("'triple'")
  local quad_code = codeTableByName_number("'quad'")
  local passthrough_code = codeTableByName_number("'passthrough'")

  printFuncSizes()

  pwd()
  xobjregister(add2_code, "add2")
  xobjregister(sub2_code, "sub2")
  xobjregister(adddouble_code, "adddouble")
  xobjregister(subdouble_code, "subdouble")
  xobjregister(triple_code, "triple")
  xobjregister(quad_code, "quad")
  xobjregister(passthrough_code, "passthrough")

  a=xcall(2,{"uint32","uint32"},"uint32",0, {30,20})
  print("call add result", a)
  a=xcall(2,{"uint32", "uint32"},"uint32",1, {30,20})
  print("call sub result", a)

  arg1 = 100
  arg2 = 200
  a=xcall(2,{"sint32", "sint32"},"sint32",1, {arg1,arg2})
  print("xcall returned:",a)

  if a ~= -100 then print("test failed") end
  a=xcall(2,{"double", "double"},"double",2, {333.333,222.222})
  print("xcall returned:",a)
  -- FIXME
  if tostring(a) ~= tostring(555.555) then print("test failed") end
  a=xcall(2,{"double", "double"},"double",3, {333.333,222.222})
  print("xcall returned:",a)
  if tostring(a) ~= tostring(111.111) then print("test failed") end

  a=xcall(3,{"double", "double", "double"},"double",4, {333.333,222.222,111.111})
  print("xcall returned:",a)
  a=xcall(3,{"double", "double", "double"},"double","triple", {333.333,222.222,111.111})
  print("xcall returned:",a)
  if tostring(a) ~= tostring(666.666) then print("test failed") end

  --a=xcall(4,{"sint32", "sint32", "sint32", "sint32"},"sint32",5, {10,20,30,40})
  --print("xcall returned:",a)
  --if a ~= 100 then print("test failed") end

  a=xcall(1,{"string"},"string","passthrough", {"i live!"})
  print("xcall returned:",a)
end

main()
--------------------------------------------------------------------------------------------------------------

