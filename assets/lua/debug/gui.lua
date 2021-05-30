local g_debugElement
console.register_command("debug_gui",function(pl,className,width,height)
	if(util.is_valid(g_debugElement)) then g_debugElement:Remove() end
	if(className == nil) then return end
	local p = gui.create(className)
	g_debugElement = p
	if(p == nil) then
		console.print_warning("Unable to create element of type '" .. className .. "'!")
		return
	end
	if(width ~= nil) then p:SetWidth(width) end
	if(height ~= nil) then p:SetHeight(height) end
	p:RequestFocus()
	p:TrapFocus()
end)
