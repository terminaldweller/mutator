
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

function asm_rewriter()
  local text_section = xobj.getTextSection()
  for k,v in pairs(text_section) do io.write(colors("%{blue}"..string.format("%02x",k)),":",colors("%{green}"..string.format("%02x",v)),"\t") end
  io.write("\n")
end

--main()
asm_rewriter()
