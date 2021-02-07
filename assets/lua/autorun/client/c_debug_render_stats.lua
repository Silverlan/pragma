local elUi
local function show_debug_render_stats_ui()
	if(util.is_valid(elUi)) then
		elUi:Remove()
		return
	end
	include("/gui/widebugrenderstats.lua")

	util.remove(elUi)
	elUi = gui.create("WIDebugRenderStats")
	elUi:SetSize(512,800)
	elUi:SetAlpha(200)
	elUi:RequestFocus()
	elUi:TrapFocus()
	elUi:SetMouseInputEnabled(true)
end

console.register_command("debug_render_stats_ui",function(pl,joystickAxisMagnitude)
	show_debug_render_stats_ui()
end)
