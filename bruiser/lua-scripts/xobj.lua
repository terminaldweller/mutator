------------------------------------------------Project Mutator-----------------------------------------------
--bruiser's xobj module
--Copyright (C) 2018 Farzad Sadeghi

--This program is free software; you can redistribute it and/or
--modify it under the terms of the GNU General Public License
--as published by the Free Software Foundation; either version 3
--of the License, or (at your option) any later version.

--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU General Public License for more details.

--You should have received a copy of the GNU General Public License
--along with this program; if not, write to the Free Software
--Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
--------------------------------------------------------------------------------------------------------------
--start of xobj module
local xobj = {}

local elf_file = ""
function xobj.getSO(so_path)
  elf_file = so_path
end

function xobj.getGlobalTable()
  local return_table = {}
  local names = objload("load", "elf_get_obj_names", elf_file, "symbol_list")
  local sizes = objload("load", "elf_get_obj_sizes", elf_file, "symbol_list")
  for i=1,#names,1 do
    return_table[names[i]] = sizes[i]
  end
  return return_table
end

function xobj.printObjNames()
  local c = objload("load", "elf_get_obj_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function xobj.printObjSizes()
  local c = objload("load", "elf_get_obj_sizes", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function xobj.printFuncNames()
  local c = objload("load", "elf_get_func_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    print(k,v)
  end
end

function xobj.printFuncCode()
  local c = objload("load", "elf_get_func_code", elf_file, "code_list")
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

function xobj.findMain()
  local c = objload("load", "elf_get_func_names", elf_file, "symbol_list")
  for k,v in ipairs(c) do
    if v == "'main'" then
      io.write("main index is".." "..k.."\n")
      return k
    end
  end
end

function xobj.codeTables()
  local return_table = {}
  local func_name_table = objload("load", "elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("load", "elf_get_func_code", elf_file, "code_list")
  for i=1,#func_name_table,1 do
    return_table[func_name_table[i]] = code_table[i]
  end
  return return_table
end

function xobj.codeTableByName(name)
  local return_table = {}
  local func_name_table = objload("load", "elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("load", "elf_get_func_code", elf_file, "code_list")
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

function xobj.codeTableByName_number(name)
  local return_table = {}
  local func_name_table = objload("load", "elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("load", "elf_get_func_code", elf_file, "code_list")
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

function xobj.printFuncSizes()
  local func_name_table = objload("load", "elf_get_func_names", elf_file, "symbol_list")
  local code_table = objload("load", "elf_get_func_code", elf_file, "code_list")
  local counter = 1
  print("function sizes:")
  for k, v in ipairs(code_table) do
    print("code size for "..func_name_table[counter].." is".." "..#v)
    counter = counter + 1
  end
end

function xobj.getTextSection(elf_exe)
  return objload("load", "elf_get_text_section", elf_exe, "bytes")
end

function xobj.getRODataSection(elf_exe)
  return objload("load", "elf_get_rodata_section", elf_exe, "bytes")
end

function xobj.CSDump(code)
  ret = ""
  for k,v in pairs(code) do
    ret = ret.."\\x"..string.format("%02x",v)
  end
  return ret
end

--end of xobj module
return xobj
--------------------------------------------------------------------------------------------------------------

