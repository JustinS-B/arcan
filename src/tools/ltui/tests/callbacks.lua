local twin = tui_open("Callbacks", "Untitled 1",
{
handlers = {

-- periodic updates
tick = function(c)
	print("tick");
end,

-- mouse cursor motion on the surface
mouse_motion = function(c, relative, x, y, mods)
	print(relative, x, y, mods);
end,

-- mouse button + last known position and input state
mouse_button = function(c, subid, active, x, y, mods)
	print(subid, active, x, y, mods);
end,

-- utf8 character input (validated), return true if consumed (blocks key)
utf8 = function(c, u8)
	print("utf8", u8);
	return false;
end,

-- single key input
key = function(c, xkeysym, scancode, mods)
	print(subid, xkeysym, scancode, scancode, mods)
end,

-- request state save (!input_mode) or state restore (input_mode)
state = function(c, file, input_mode)
end,

-- input string paste, cont is set if the message is multipart
paste = function(c, string, cont)
	print("paste", string, cont);
end,

-- window will be resized
resize = function(c, col, row, neww, newh)
	print("resize", col, row, neww, newh);
end,

}
});

while (true) do
	twin:process();
end
