-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

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
