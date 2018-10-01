
colors = require("ansicolors")

local libwasm = {}

function libwasm.demo(wasm_path)
  local a = getwasmobj(wasm_path)
  print(a)
  print(type(a))
  for k, v in pairs(a) do
    print(k, v, type(k), type(v))
  end
  libwasm.dump_import_section(a)
  libwasm.dump_type_section(a)
  libwasm.dump_function_section(a)
  libwasm.dump_table_section(a)
  libwasm.dump_memory_section(a)
  libwasm.dump_global_section(a)
  libwasm.dump_export_section(a)
  libwasm.dump_start_section(a)
  libwasm.dump_element_section(a)
  libwasm.dump_code_section(a)
  libwasm.dump_data_section(a)
end

function libwasm.dump_type_section(a)
  print(colors("%{green}".."\ntype section:"))
  if a["type_section"] ~= nil then
    io.write(tostring("id:"..a["type_section"]:id()).."\n")
    io.write(tostring("payloadlength:"..a["type_section"]:payloadlength()).."\n")
    io.write(tostring("namelength:"..a["type_section"]:namelength()).."\n")
    io.write(tostring("name:"..a["type_section"]:name()).."\n")
    io.write(tostring("count:"..a["type_section"]:count()).."\n")
    io.write("entries"..tostring(a["type_section"]:entries()).."\n")
    for k, v in pairs(a["type_section"]:entries()) do
      io.write(v:form().."\t")
      io.write(v:param_count().."\t")
      io.write(v:param_types().."\t")
      io.write(v:return_count().."\t")
      io.write(v:return_types().."\n")
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_import_section(a)
  print(colors("%{green}".."\nimport section:"))
  if a["import_section"] ~= nil then
    io.write("id:"..tostring(a["import_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["import_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["import_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["import_section"]:name()).."\n")
    io.write("count:"..tostring(a["import_section"]:count()).."\n")
    io.write("entries"..tostring(a["import_section"]:entries()).."\n")
    for k, v in pairs(a["import_section"]:entries()) do
      --print(k, v, type(v))
      io.write("module length:"..v:module_length().."\t")
      io.write("module str:"..v:module_str().."\t")
      io.write("field len:"..v:field_len().."\t")
      io.write("field str:"..v:field_str().."\t")
      io.write("kind:"..v:kind().."\t")
      io.write("type:"..v:type().."\n")
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_function_section(a)
  print(colors("%{green}".."\nfunction section:"))
  if a["function_section"] ~= nil then
    io.write("id:"..tostring(a["function_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["function_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["function_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["function_section"]:name()).."\n")
    io.write("count:"..tostring(a["function_section"]:count()).."\n")
    io.write("types:"..tostring(a["function_section"]:types()).."\n")
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_table_section(a)
  print(colors("%{green}".."\ntable section:"))
  if a["table_section"] ~= nil then
    io.write("id:"..tostring(a["table_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["table_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["table_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["table_section"]:name()).."\n")
    io.write("count:"..tostring(a["table_section"]:count()).."\n")
    io.write("entries:"..tostring(a["table_section"]:entries()).."\n")
    for k, v in pairs(a["table_section"]:entries()) do
      io.write(v:element_type().."\t")
      io.write(tostring(v:resizable_limit()).."\t")
      io.write(v:resizable_limit():flags().."\t")
      io.write(v:resizable_limit():initial().."\t")
      io.write(v:resizable_limit():maximum().."\n")
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end


function libwasm.dump_memory_section(a)
  print(colors("%{green}".."\nmemory section:"))
  if a["memory_section"] ~= nil then
    io.write("id:"..tostring(a["memory_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["memory_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["memory_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["memory_section"]:name()).."\n")
    --for wasm v.1.0. memory section count is 1
    --io.write("count:"..tostring(a["memory_section"]:count()).."\n")
    io.write("entries:"..tostring(a["memory_section"]:entries()).."\n")
    io.write(a["memory_section"]:entries():resizable_limit():flags().."\t")
    io.write(a["memory_section"]:entries():resizable_limit():initial().."\t")
    io.write(a["memory_section"]:entries():resizable_limit():maximum().."\n")
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_global_section(a)
  print(colors("%{green}".."\nglobal section:"))
  if (a["global_section"] ~= nil) then
    io.write("id:"..tostring(a["global_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["global_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["global_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["global_section"]:name()).."\n")
    io.write("count:"..tostring(a["global_section"]:count()).."\n")
    io.write("globals:"..tostring(a["global_section"]:globals()).."\n")
    for k, v in pairs(a["global_section"]:globals()) do
      io.write(v:global_type().."\t")
      io.write(v:init().."\n")
      io.write(v:init():code().."\n")
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_export_section(a)
  print(colors("%{green}".."\nexport section:"))
  if (a["export_section"] ~= nil) then
    io.write("id:"..tostring(a["export_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["export_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["export_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["export_section"]:name()).."\n")
    io.write("count:"..tostring(a["export_section"]:count()).."\n")
    io.write("entries:"..tostring(a["export_section"]:entries()).."\n")
    for k, v in pairs(a["export_section"]:entries()) do
      io.write(v:field_len().."\t")
      io.write(v:field_str().."\t")
      io.write(v:kind().."\t")
      io.write(v:index().."\n")
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_start_section(a)
  print(colors("%{green}".."\nstart section:"))
  if (a["start_section"] ~= nil) then
    io.write("id:"..tostring(a["start_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["start_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["start_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["start_section"]:name()).."\n")
    io.write("index:"..tostring(a["start_section"]:index()).."\n")
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_element_section(a)
  print(colors("%{green}".."\nelement section:"))
  if (a["element_section"] ~= nil) then
    io.write("id:"..tostring(a["element_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["element_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["element_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["element_section"]:name()).."\n")
    io.write("count:"..tostring(a["element_section"]:count()).."\n")
    io.write(colors("%{cyan}".."entries:"..tostring(a["element_section"]:entries()).."\n"))
    for k, v in pairs(a["element_section"]:entries()) do
      io.write("index:"..v:index().."\t")
      io.write("init:")
      for i = 1, #v:init():code() do
        local c = v:init():code():sub(i,i)
        io.write(colors("%{red}"..string.byte(c)).." ")
      end
      io.write("\n")
      io.write("number of elements:"..v:num_length().."\t")
      io.write("elems:")
      for i, j in pairs(v:elems()) do
        io.write(colors("%{red}"..j.." "))
      end
      io.write("\n")
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_code_section(a)
  print(colors("%{green}".."\ncode section:"))
  if (a["code_section"] ~= nil) then
    io.write("id:"..tostring(a["code_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["code_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["code_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["code_section"]:name()).."\n")
    io.write("count:"..tostring(a["code_section"]:count()).."\n")
    io.write("bodies:"..tostring(a["code_section"]:bodies()).."\n")
    for k,v in pairs(a["code_section"]:bodies()) do
      io.write(colors("\n%{cyan}".."entry:\n"))
      io.write("body_size:"..v:body_size().."\t")
      io.write("local_count:"..v:local_count().."\t")
      io.write(tostring(v:locals()).."\t")
      for i, j in pairs(v:locals()) do
        io.write("locals count:"..j:count().."\t")
        io.write("locals type:"..j:type().."\t")
      end
      io.write("function body:\n")
      for i,j in pairs(v:code()) do
        io.write(colors("%{red}"..j.." "))
      end
        io.write("\n")
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_data_section(a)
  print(colors("%{green}".."\ndata section:"))
  if (a["data_section"] ~= nil) then
    io.write("id:"..tostring(a["data_section"]:id()).."\n")
    io.write("payloadlength:"..tostring(a["data_section"]:payloadlength()).."\n")
    io.write("namelength:"..tostring(a["data_section"]:namelength()).."\n")
    io.write("name:"..tostring(a["data_section"]:name()).."\n")
    io.write("count:"..tostring(a["data_section"]:count()).."\n")
    io.write("entries:"..tostring(a["data_section"]:entries()).."\n")
    if type(a["data_section"]:entries()) == "table" then
      io.write(colors("%{cyan}".."entries:").."\n")
      for k,v in pairs(a["data_section"]:entries()) do
        io.write("index:"..v:index().."\n")
        io.write("offset: ")
        for i = 1, #v:offset():code() do
          local c = v:offset():code():sub(i,i)
          io.write(colors("%{red}"..string.byte(c)).." ")
        end
        io.write("\n")
        io.write("size:"..v:size().."\n")
        for i, j in pairs(v:data()) do
          io.write(colors("%{blue}"..string.char(j)))
        end
        io.write("\n")
      end
    end
  else
    print(colors("%{red}".."section doesnt exist."))
  end
end

function libwasm.dump_all(wasm_path)
  local a = getwasmobj(wasm_path)
  libwasm.dump_import_section(a)
  libwasm.dump_type_section(a)
  libwasm.dump_function_section(a)
  libwasm.dump_table_section(a)
  libwasm.dump_memory_section(a)
  libwasm.dump_global_section(a)
  libwasm.dump_export_section(a)
  libwasm.dump_start_section(a)
  libwasm.dump_element_section(a)
  libwasm.dump_code_section(a)
  libwasm.dump_data_section(a)
end

function libwasm.dev(wasm_path)
  local a = getwasmobj(wasm_path)
  libwasm.dump_import_section(a)
end

--libwasm.dev("/home/bloodstalker/devi/hell2/bruiser/autogen/wasm/ft/test.wasm")
libwasm.demo("/home/bloodstalker/devi/hell2/bruiser/autogen/wasm/ft/test.wasm")
--libwasm.dump_all("/home/bloodstalker/devi/hell2/bruiser/autogen/wasm/ft/test.wasm")
return libwasm
