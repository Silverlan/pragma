-- SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local models = { "cube", "empty", "error", "player/soldier", "maps/empty_sky/skybox_3" }

for _, mdlName in ipairs(models) do
	local mdl = game.load_model(mdlName)
	if mdl == nil then
		return false, "Failed to load model '" .. mdlName .. "'!"
	end
end

return true
