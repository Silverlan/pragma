-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include_component("gm_generic")
util.register_class("ents.GmZen", BaseEntityComponent)
local Component = ents.GmZen

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:AddEntityComponent("gm_generic")
	if CLIENT then
		self:BindEvent(ents.GamemodeComponent.EVENT_ON_GAME_READY, "InitializeHud")
	end
end
ents.register_component("gm_zen", Component, "gameplay")
