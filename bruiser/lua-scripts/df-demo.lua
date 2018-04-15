
xobj = require("lua-scripts.xobj")
asmrw = require("lua-scripts.asmrw")
colors = require("ansicolors")

df_exe = "/home/bloodstalker/df/df_44_09_linux/df_linux/libs/Dwarf_Fortress"

function main()
  local text_section = xobj.getTextSection(df_exe)
end

function pretty_dump()
  count = 0
  local text_section = xobj.getTextSection(df_exe)
  io.write(colors("%{blue}".."    ".."\t".."00 ".."01 ".."02 ".."03 ".."04 ".."05 ".."06 ".."07 ".."08 ".."09 ".."0A ".."0B ".."0C ".."0D ".."0E ".."0F"))
  for k,v in pairs(text_section) do
    if count % 16 == 0 then
      print()
      io.write(colors("%{blue}".."0x"..string.format("%03x",count)), "\t")
    end
    io.write(colors("%{green}"..string.format("%02x", v)), " ")
    count = count + 1
  end
  count = 0
  print()
end

function jmp_table_test()
  io.write(colors("%{cyan}".."lua:getting text section...\n"))
  local text_section = xobj.getTextSection(df_exe)
  io.write(colors("%{green}".."lua:got text section.\n"))
  local head = jmp_s_t()
  -- messes up the stack. I could fix it but not sure why i would want to keep this in
  --local head2 = jmp_s_t:new()
  io.write(colors("%{cyan}".."lua:calling getjmptable\n"))
  head = getjmptable(#text_section, text_section)

  while head:inext() ~= nil do
    head:dump("entry")
    io.write("type:", head:type(), "\tlocation:", "0x"..string.format("%x", head:location()))
    print()
    head = head:inext()
  end
end

--main()
--pretty_dump()
jmp_table_test()
