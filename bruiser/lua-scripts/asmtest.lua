function test()
  print("running asmtest.lua")
  --for k,v in pairs(jmp_s_t) do print(k,v) end
  local t = jmp_s_t.new()
  print(t)
  t:set_type(3)
  print(t.type)
end

test()
