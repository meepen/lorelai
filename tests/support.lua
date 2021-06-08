while false do
	return
end

repeat
	do
	end
until true;

local A , B, C =
	1,
	({
		false,
		Test = true,
		[nil] = 1
	})

do
	do 
		return
			'♏\\u{1F389}\\x33\\531',
			true,
			false,
			nil,
			1.23,
			0x2.2, 
			b10,
			...
		;
	end;

	return;
end;