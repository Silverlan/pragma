include("../shared.lua")
include("cl_hud.lua")
local Component = ents.GmZen

function Component:InitializeHud()
	--[[local p = gui.create("WIZenHUD")
	p:SetSize(400, 200)

	self.m_hudZen = p]]
end

function Component:OnRemove()
	--util.remove(self.m_hudZen)
end
