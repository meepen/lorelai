local t = {1, [false] = true, [true] = false}

t.a = "lmao"

print(t[1], t[true], t[false], t.a)
print(t[1], t[true], t[false], t.a)

local function fn()
	print(1)
end

local CONST, NOTCONST = 1, 2
local SECONDCONST = CONST * 2
local SECONDNOTCONST = NOTCONST * 2
do
    NOTCONST = 4
end

print(CONST, NOTCONST)
print(SECONDCONST, SECONDNOTCONST)

fn()