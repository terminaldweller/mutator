
xobj = require("xobj")
asmrw = require("asmrw")
colors = require("ansicolors")

function df_demo()
  df_exe = "/home/bloodstalker/df/df_44_09_linux/df_linux/libs/Dwarf_Fortress"
  --asmrw.strings(df_exe)
  pgrep_handle = io.popen("pgrep Dwarf_Fortress")
  for pid in pgrep_handle:lines() do
    df_pid = tonumber(pid)
  end
  print("df pid is "..tostring(df_pid))
  ramdump(df_pid, "/tmp/ramdump")
  -- @DEVI-broken
  --ramdump(11419, "/tmp/ramdump")
  ram = io.open("/tmp/ramdump", "r")
  io.input(ram)
  print(io.read())
end

df_demo()
