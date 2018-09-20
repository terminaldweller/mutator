
function demo4()
  local a = getwasmobj("/home/bloodstalker/devi/hell2/bruiser/autogen/wasm/ft/test.wasm")
  print(a)
  print(type(a))
  for k, v in pairs(a) do
    print(k, v, type(k), type(v))
  end
  print("type section:")
  io.write(tostring(a["type_section"]:id()).."\n")
  io.write(tostring(a["type_section"]:payloadlength()).."\n")
  io.write(tostring(a["type_section"]:namelength()).."\n")
  io.write(tostring(a["type_section"]:name()).."\n")
  io.write(tostring(a["type_section"]:count()).."\n")
  io.write(tostring(a["type_section"]:entries()).."\n")
end

demo4()
