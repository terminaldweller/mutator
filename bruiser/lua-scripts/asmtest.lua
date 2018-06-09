
xobj = require("xobj")
asmrw = require("asmrw")

function test()
  local elf_exe = "../bfd/test/test"
  local text_section = xobj.getTextSection(elf_exe)
  local head = jmp_s_t()
  -- messes up the stack. I could fix it but not sure why i would want to keep this in
  --local head2 = jmp_s_t:new()
  head = getjmptable(#text_section, text_section)

  while head:inext() ~= nil do
    head:dump("entry")
    io.write("type:", head:type(), "\tlocation:", "0x"..string.format("%x", head:location()))
    print()
    head = head:inext()
  end
end

test()
