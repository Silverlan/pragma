-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include("../shared.lua")

function ents.WeaponPhyscannon:InitializeModel(component)
	component:SetModel(self.WorldModel)
end
