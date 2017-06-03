in_command = io.read()
if in_command == "list funcs" then
  clangreturn = Funcs()
elseif in_command == "list vars" then
  clangreturn = Vars()
elseif in_command == "list structs" then
  clangreturn = Structs()
elseif in_command == "list arrays" then
  clangreturn = Arrayss()
elseif in_command == "list classes" then
  clangreturn = Classes()
elseif in_command == "list unions" then
  clangreturn = Unions()
end
