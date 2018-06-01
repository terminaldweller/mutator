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
local pack_name = ...
local Demo1 = {}
elf_file = "/home/bloodstalker/devi/hell2/bfd/test/test.so"
--elf_file = "/home/bloodstalker/devi/hell2/bfd/test/test.so"
--elf_file = "../bfd/test/test"

function Demo1.getGlobalTable()
  local return_table = {}
  local names = objload("elf_get_obj_names", elf_file, "symbol_list")
  local sizes = objload("elf_get_obj_sizes", elf_file, "bytes")
  for i=1,#names,1 do
    return_table[names[i]] = sizes[i]
  end
  return return_table
end

function Demo1.printObjNames()
  local c = objload("elf_get_obj_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function Demo1.printObjSizes()
  local c = objload("elf_get_obj_sizes", elf_file, "bytes")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function Demo1.printFuncNames()
  local c = objload("elf_get_func_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function Demo1.printFuncCode()
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

function Demo1.findMain()
  local c = objload("elf_get_func_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    if v == "main" then 
      io.write("main index is".." "..k.."\n")
      return k
    end
  end
end

function Demo1.codeTables()
  local return_table = {}
  local func_name_table = objload("elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("elf_get_func_code", elf_file, "code_list")
  for i=1,#func_name_table,1 do
    return_table[func_name_table[i]] = code_table[i]
  end
  return return_table
end

function Demo1.codeTableByName(name)
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

function Demo1.codeTableByName_number(name)
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

function Demo1.printFuncSizes()
  local func_name_table = objload("elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("elf_get_func_code", elf_file, "code_list")
  local counter = 1
  print("function sizes:")
  for k, v in ipairs(code_table) do
    print("code size for "..func_name_table[counter].." is".." "..#v)
    counter = counter + 1
  end
end

function Demo1.demo1()
  pwd()
  Demo1.printObjNames()
  Demo1.printObjSizes()
  Demo1.printFuncNames()
  Demo1.printFuncCode()
  Demo1.findMain()

  local code_table = Demo1.codeTables()
  print(code_table["'main'"])
  for k,v in ipairs(code_table["'main'"]) do
    io.write(string.format('%02x', v), " ")
  end
  io.write("\n")
local C_main_code = Demo1.codeTableByName("'main'")
  for k, v in ipairs(C_main_code) do
    io.write(v, " ")
  end
  io.write("\n")

  local add2_code = Demo1.codeTableByName_number("'add2'")
  local sub2_code = Demo1.codeTableByName_number("'sub2'")
  local adddouble_code = Demo1.codeTableByName_number("'adddouble'")
  local subdouble_code = Demo1.codeTableByName_number("'subdouble'")
  local triple_code = Demo1.codeTableByName_number("'triple'")
  local quad_code = Demo1.codeTableByName_number("'quad'")
  local passthrough_code = Demo1.codeTableByName_number("'passthrough'")

  Demo1.printFuncSizes()

  print("passthrough_code: ")
  for k,v in pairs(passthrough_code) do
    io.write(v," ")
  end
  io.write("\n")


  print("xsize = "..xsize())
  xobjregister(add2_code, "add2")
  xobjregister(sub2_code, "sub2")
  xobjregister(adddouble_code, "adddouble")
  xobjregister(subdouble_code, "subdouble")
  xobjregister(triple_code, "triple")
  xobjregister(quad_code, "quad")
  xobjregister(passthrough_code, "passthrough")
  print("xsize = "..xsize())

  local x_list = xobjlist()
  for k,v in pairs(x_list) do
    print(k,v)
  end

  a=xcall(2,{"uint32","uint32"},"uint32",0, {30,20})
  print("call add result", a)
  a=xcall(2,{"uint32", "uint32"},"uint32",1, {30,20})
  print("call sub result", a)

  arg1 = 100
  arg2 = 200
  a=xcall(2,{"sint32", "sint32"},"sint32","sub2", {arg1,arg2})
  print("xcall returned:",a)

  if a ~= -100 then print("test failed") end
  a=xcall(2,{"double", "double"},"double",2, {333.333,222.222})
  print("xcall returned:",a)
  if tostring(a) ~= tostring(555.555) then print("test failed"); os.exit(1) end
  a=xcall(2,{"double", "double"},"double",3, {333.333,222.222})
  print("xcall returned:",a)
  if tostring(a) ~= tostring(111.111) then print("test failed"); os.exit(1) end

  a=xcall(3,{"double", "double", "double"},"double",4, {333.333,222.222,111.111})
  print("xcall returned:",a)
  a=xcall(3,{"double", "double", "double"},"double","triple", {333.333,222.222,111.111})
  print("xcall returned:",a)
  if tostring(a) ~= tostring(666.666) then print("test failed"); os.exit(1) end

  a=xcall(1,{"string"},"string","passthrough", {"i live!"})
  print("xcall returned:",a)
  if (a ~= "i live!") then print("test failed"); os.exit(1) end

  -- nested call
  --a=xcall(4,{"sint32", "sint32", "sint32", "sint32"},"sint32",5, {10,20,30,40})
  --print("xcall returned:",a)
  --if a ~= 100 then print("test failed") end

  a = xobjlist()
  print("the offset of quad and add2 is : ", a["quad"] - a["add2"])

  mem_size = xmemusage()
  print("memory used "..mem_size)
  xclear()
  mem_size = xmemusage()
  print("memory used "..mem_size)

end

if type(package.loaded[pack_name]) ~= "userdata" then
  Demo1.demo1()
else
  return Demo1
end
--------------------------------------------------------------------------------------------------------------

