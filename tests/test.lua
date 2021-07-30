
print(0 / 0, 1 / 0, -1 / 0)

local function fn()
end

local CONST, NOTCONST = 1, 2
local SECONDCONST = CONST * 2
do
    NOTCONST = 4
end
local SECONDNOTCONST = NOTCONST * 3

print(CONST, NOTCONST)
print(SECONDCONST, SECONDNOTCONST)
