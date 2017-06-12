print(Vars())
print(Funcs())
print(showsource(1,5,"test.cpp"))

a={showsource(1,5,"test.cpp")}
for k,v in pairs(a) do
  print(k,v)
end

b={Vars()}
for k,v in pairs(b) do
  print(k,v)
end

print("today is:")
print(os.date())
