local a = -3.14, (false ~= not true), true, nil, 333, 'bbbd', ('b').c, d['e'].f(), g:h(i)
a, b, c().d = 4, "B"
print(1)

do
	local b, a = 1, 2
end

print(a)

a = 0

repeat
	a = a + 1
until a == 2

if true then
	itistrue()
elseif true then
	itistrue2()
else
	itisfalse()
end

while (a) do
	a = false
	print(a, tailcall())
	break
end

for i in pairs "a" do
	print(i)
end

for i = 1, 2, 0.5 do
	print(i)
end

return a, i, 1, 2, q()