
xobj = require("lua-scripts.xobj")
colors = require("ansicolors")
elf_file = "../bfd/test/test.so"
elf_exe = "../bfd/test/test"

function main()
  xobj.getSO(elf_file)
  local add2_code = xobj.codeTableByName_number("'add2'")
  local sub2_code = xobj.codeTableByName_number("'sub2'")
  local adddouble_code = xobj.codeTableByName_number("'adddouble'")
  local subdouble_code = xobj.codeTableByName_number("'subdouble'")
  local triple_code = xobj.codeTableByName_number("'triple'")
  local quad_code = xobj.codeTableByName_number("'quad'")
  local passthrough_code = xobj.codeTableByName_number("'passthrough'")

  --xobj.printFuncSizes()

  xobjregister(add2_code, "add2")
  xobjregister(sub2_code, "sub2")
  xobjregister(adddouble_code, "adddouble")
  xobjregister(subdouble_code, "subdouble")
  xobjregister(triple_code, "triple")
  xobjregister(quad_code, "quad")
  xobjregister(passthrough_code, "passthrough")
end

function pretty_dump()
  count = 0
  local text_section = xobj.getTextSection()
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

function test()
  local text_section = xobj.getTextSection()
  dummy = xobj.CSDump(text_section)
  print(dummy)
end

function asm_rewriter()
  local text_section = xobj.getTextSection()
  local head = getjmptable(#text_section, text_section)
  print("head value is",head)
  dumpjmptable(head)
  freejmptable(haed)
end

function jmp_t_test()
  for k,v in pairs(jmp_s_t) do print(k,v) end
  local t = jmp_s_t.new()
end

--main()
--pretty_dump()
--test()
--asm_rewriter()
jmp_t_test()
