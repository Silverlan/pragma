--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Component = ents.LightmapBaker

function Component:WriteLightmapInformationToMap(mapFilePath, mapName, lightmapDataCache, lightmapMaterial)
	local worldData, err = util.WorldData.load(mapFilePath)
	if worldData == nil then
		self:LogErr("Failed to load world data '{}': {}", mapFilePath, err)
		return false
	end
	local entWorld = worldData:FindWorld()
	if entWorld == nil then
		self:LogErr("No world entity exists in world data!")
		return false
	end
	local lightmapDataCacheC = entWorld:AddComponent("light_map_data_cache")
	local data = lightmapDataCacheC:GetData()
	data:SetValue("lightmapDataCache", udm.TYPE_STRING, lightmapDataCache)

	local lightmapC = entWorld:AddComponent("light_map")
	local data = lightmapC:GetData()
	data:SetValue("lightmapMaterial", udm.TYPE_STRING, lightmapMaterial)

	entWorld:AddComponent("light_map_receiver")
	local res, err = worldData:Save(mapFilePath, mapName)
	if res == false then
		self:LogErr("Failed to save world data as '{}': {}", mapFilePath, err)
		return false
	end
	return true
end

function Component:BakeMapLightmaps()
	local mapName = game.get_map_name()
	local fileName = asset.find_file(mapName, asset.TYPE_MAP)
	local absFilePath = (fileName ~= nil) and asset.relative_path_to_absolute_path(fileName, asset.TYPE_MAP) or nil
	if absFilePath == nil then
		self:LogErr("Failed to determine file location of map '{}'!", mapName)
		return false
	end

	local ent = ents.get_world()
	if ent == nil then
		self:LogErr("No world entity found!")
		return false
	end

	if self:GenerateLightmapUvs() == false then
		self:LogErr("Failed to generate lightmap UVs.")
		return false
	end
	local dirLightmapJob = self:GenerateDirectionalLightmaps()
	if dirLightmapJob == nil then
		self:LogErr("Failed to generate directional lightmaps.")
		return false
	end
	local lightmapJob = self:GenerateLightmaps()
	if lightmapJob == false then
		self:LogErr("Failed to generate lightmaps.")
		return false
	end

	local lightmapDataCacheC = ent:GetComponent(ents.COMPONENT_LIGHT_MAP_DATA_CACHE)
	if lightmapDataCacheC == nil then
		self:LogErr("No lightmap data cache.")
		return false
	end

	local lightmapC = ent:GetComponent(ents.COMPONENT_LIGHT_MAP)
	if lightmapC == nil then
		self:LogErr("No lightmap component.")
		return false
	end
	local lightmapDataCache = lightmapDataCacheC:GetLightMapDataCacheFilePath()
	local lightmapMaterial = lightmapC:GetLightmapMaterialName()
	return self:WriteLightmapInformationToMap(absFilePath, mapName, lightmapDataCache, lightmapMaterial)
end
