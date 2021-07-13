local t = {1, [false] = true, [true] = false}

t.a = "lmao"

print(t[1], t[true], t[false], t.a)
print(t[1], t[true], t[false], t.a)

local function a()
	print(1)
end

a()