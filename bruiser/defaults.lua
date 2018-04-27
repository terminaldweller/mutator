-- bruiser default script.
-- This is run everytime bruiser is called.

-- adds luarocks' path and cpath to bruiser
function default_luarocks_modules()
  local luarocks_handle = io.popen("luarocks path --bin")
  local path_b = false
  local cpath_b = false
  for line in luarocks_handle:lines() do
    local path = string.match(line, "LUA_PATH%s*=%s*('.+')")
    local cpath = string.match(line, "LUA_CPATH%s*=%s*('.+')")
    if path ~= nil then 
      package.path = package.path..";"..path
    end
    if cpath ~= nil then
      package.cpath = package.cpath..";"..cpath
    end
  end

  if path_b then
    io.write("failed to get path from luarocks.\n")
  end
  if cpath_b then
    io.write("failed to get cpath from luarocks.\n")
  end
end

default_luarocks_modules()
