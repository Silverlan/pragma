local statsWindow
local function show_debug_render_stats_ui()
	if util.is_valid(statsWindow) then
		statsWindow:Close()
		statsWindow = nil
		return
	end
	include("/gui/widebugrenderstats.lua")

	util.remove(statsWindow)

	local width = 512
	local height = 800

	local createInfo = prosper.WindowCreateInfo()
	createInfo.width = width
	createInfo.height = height
	createInfo.title = "Render Stats"
	local windowHandle = prosper.create_window(createInfo)
	if windowHandle == nil then
		return
	end
	statsWindow = windowHandle
	local el = gui.get_base_element(windowHandle)
	if util.is_valid(el) == false then
		return
	end

	local elUi = gui.create("WIDebugRenderStats", el, 0, 0, width, height, 0, 0, 1, 1)
	elUi:SetAlpha(200)
	elUi:RequestFocus()
	elUi:TrapFocus()
	elUi:SetMouseInputEnabled(true)
end

console.register_command("debug_render_stats_ui", function(pl, joystickAxisMagnitude)
	show_debug_render_stats_ui()
end)
