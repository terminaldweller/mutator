
xobj = require("xobj")
asmrw = require("asmrw")
colors = require("ansicolors")

df_exe = "/home/bloodstalker/df/df_44_09_linux/df_linux/libs/Dwarf_Fortress"

function jmp_table_test()
  io.write(colors("%{cyan}".."lua:getting text section...\n"))
  local text_section = xobj.getTextSection(df_exe)
  local rodata = xobj.getRODataSection(df_exe)
  for k,v in pairs(rodata) do
    if v > 33 and v < 127 then 
      io.write(string.format("%c",v)) 
    else
      io.write(" ")
    end
  end
  io.write("\0\n")
end

asmrw.strings(df_exe)
