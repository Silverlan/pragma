--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local cbOnBakingComplete
console.register_command("map_bake_lightmaps", function(pl, ...)
	include("/pfm/pfm.lua")
	include("/util/lightmap_bake.lua")
	include("/pfm/bake/lightmaps.lua")

	local ent = ents.get_world()
	if ent == nil then
		console.print_warning("No world entity found!")
		return
	end
	local cmdArgs = console.parse_command_arguments({ ... })

	local function getFirstArg(param)
		if cmdArgs[param] == nil then
			return
		end
		return cmdArgs[param][1]
	end

	local shutdownOnComplete = false
	if cmdArgs["shutdown"] ~= nil then
		if cmdArgs["shutdown"][1] == nil then
			shutdownOnComplete = true
		else
			shutdownOnComplete = toboolean(cmdArgs["shutdown"][1])
		end
	end

	local pfmBakedLightC = ent:AddComponent("lightmap_baker")
	cbOnBakingComplete = pfmBakedLightC:AddEventCallback(ents.LightmapBaker.EVENT_ON_BAKING_COMPLETED, function()
		if shutdownOnComplete then
			engine.shutdown()
		end
		util.remove(cbOnBakingComplete)
	end)
	pfmBakedLightC:SetWidth(toint(getFirstArg("width") or 2048))
	pfmBakedLightC:SetHeight(toint(getFirstArg("height") or 2048))
	pfmBakedLightC:SetSampleCount(toint(getFirstArg("samples") or 20000))

	if pfmBakedLightC:BakeMapLightmaps() == false then
		engine.shutdown()
	end
end)
