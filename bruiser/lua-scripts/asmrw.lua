------------------------------------------------Project Mutator-----------------------------------------------
--bruiser's asmrw module
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
--start of asmrewriter module
local asmrw = {}
xobj = require("lua-scripts.xobj")

setmetatable(jmp_s_t, {__call = 
    function(self, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12)
      local t = self.new(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12)
      print("created", t)
      return t
    end
  }
)

function jmp_s_t:dump(msg)
  print(msg, self:custom())
  return self
end

function asmrw.get_head(elf_exe)
  local text_section = xobj.getTextSection(elf_exe)
  local head = getjmptable(#text_section, text_section)
  return head
end

function asmrw.get_jmp(location)
  while head:inext() ~= nil do
    if head:location() == location then return head end
    head = head:inext()
  end
end

--end of asmrewriter module
return asmrw
--------------------------------------------------------------------------------------------------------------

