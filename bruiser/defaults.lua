-- bruiser default script.
-- This is run everytime bruiser is called.
-- By default bruiser will look for a file named defaults.lua in the same
-- directory as its executable. you can change the file using the --luadefault
-- option.

-- adds luarocks' path and cpath to bruiser
function default_luarocks_modules()
  local luarocks_handle = io.popen("luarocks path --bin")
  local path_b = false
  local cpath_b = false
  for line in luarocks_handle:lines() do
    local path = string.match(line, "LUA_PATH%s*=%s*('.+')")
    local cpath = string.match(line, "LUA_CPATH%s*=%s*('.+')")
    if path ~= nil then
      package.path = package.path..";"..string.sub(path, 2, -2)
    end
    if cpath ~= nil then
      package.cpath = package.cpath..";"..string.sub(cpath, 2, -2)
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
