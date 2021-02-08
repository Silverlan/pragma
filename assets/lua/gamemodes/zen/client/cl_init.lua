include("/gamemodes/generic/client/cl_init.lua")
include("../shared.lua")
include("cl_hud.lua")
local Component = ents.GmZen

function Component:InitializeHud()
	local p = gui.create("WIZenHUD")
	p:SetSize(400,200)
end
