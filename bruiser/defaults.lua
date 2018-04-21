-- bruiser default script.
-- This is run everytime bruiser is called.

-- adds luarocks' path and cpath to bruiser
local luarocks_handle = io.popen("luarocks path --bin")
for line in luarocks_handle:lines() do
  local path = string.match(line, "LUA_PATH%s*=%s*('.+')")
  local cpath = string.match(line, "LUA_CPATH%s*=%s*('.+')")
  if path ~= nil then 
    package.path = package.path..path
  end
  if cpath ~= nil then
    package.cpath = package.cpath..cpath
  end
end
