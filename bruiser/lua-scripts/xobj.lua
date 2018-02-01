------------------------------------------------Project Mutator-----------------------------------------------
--Copyright (C) 2018 Farzad Sadeghi

--This program is free software; you can redistribute it and/or
--modify it under the terms of the GNU General Public License
--as published by the Free Software Foundation; either version 2
--of the License, or (at your option) any later version.

--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU General Public License for more details.

--You should have received a copy of the GNU General Public License
--along with this program; if not, write to the Free Software
--Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
--------------------------------------------------------------------------------------------------------------
function getSO(so_path)
  elf_file = so_path
end

function getGlobalTable()
  local return_table = {}
  local names = objload("elf_get_obj_names", elf_file, "symbol_list")
  local sizes = objload("elf_get_obj_sizes", elf_file, "symbol_list")
  for i=1,#names,1 do
    return_table[names[i]] = sizes[i]
  end
  return return_table
end

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
--------------------------------------------------------------------------------------------------------------

