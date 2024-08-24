--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("uv_atlas_generator.lua")
include("lightmap_cache.lua")

pfm.register_log_category("lightmap")

local function generate_uv_atlas(entLm, origin, tEnts, lightmapCachePath, meshFilter)
	log.msg("Generating lightmap UV atlas...", pfm.LOG_CATEGORY_LIGHTMAP)
	local generator = util.UVAtlasGenerator(entLm:GetUuid())
	for _, ent in ipairs(tEnts) do
		if ent:IsWorld() and origin ~= nil then
			local ent = ents.get_world()
			local mdl = ent:GetModel()
			local worldC = ent:GetComponent(ents.COMPONENT_WORLD)
			local bspTree = (worldC ~= nil) and worldC:GetBSPTree() or nil
			if mdl ~= nil and bspTree ~= nil then
				local leafNode = bspTree:FindLeafNode(origin)
				if leafNode ~= nil then
					log.msg(
						"Including world entity '" .. tostring(ent) .. "' for UV atlas...",
						pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP
					)
					generator:AddEntity(ent, function(mesh, subMesh)
						if meshFilter ~= nil and meshFilter(ent, mesh, subMesh) == false then
							return false
						end
						return bspTree:IsClusterVisible(leafNode:GetCluster(), mesh:GetReferenceId())
					end)
				end
			end
		else
			log.msg(
				"Including entity '" .. tostring(ent) .. "' for UV atlas...",
				pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP
			)
			local f
			if meshFilter ~= nil then
				f = function(mesh, subMesh)
					return meshFilter(ent, mesh, subMesh)
				end
			end
			generator:AddEntity(ent, f)
		end
	end
	generator:Generate(lightmapCachePath)
	log.msg("Done!", pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP)
end

local function update_lightmap_data(tEnts)
	log.msg("Updating lightmap data...", pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP)
	for _, ent in ipairs(ents.get_all()) do
		ent:RemoveComponent(ents.COMPONENT_LIGHT_MAP_RECEIVER)
	end

	for _, ent in ipairs(tEnts) do
		local lightMapReceiver = ent:AddComponent(ents.COMPONENT_LIGHT_MAP_RECEIVER)
		lightMapReceiver:UpdateLightmapUvData()
	end

	local ent = ents.iterator({ ents.IteratorFilterComponent(ents.COMPONENT_LIGHT_MAP) })()
	if ent ~= nil then
		local lightmapC = ent:GetComponent(ents.COMPONENT_LIGHT_MAP)
		lightmapC:ReloadLightmapData()
	end
end

util.bake_lightmap_uvs = function(entLm, tEnts, lightmapCachePath, origin, meshFilter)
	log.msg("Baking lightmap UV data...", pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP)
	generate_uv_atlas(entLm, origin, tEnts, lightmapCachePath, meshFilter)
	--update_lightmap_data(tEnts)

	-- Lightmap uv cache
	--[[local success = util.save_lightmap_uv_cache(lightmapCachePath,tEnts)
	if(success) then log.msg("Successfully saved lightmap UV cache as '" .. lightmapCachePath .. "'!",pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP)
	else log.msg("Failed to save lightmap UV cache as '" .. lightmapCachePath .. "'!",pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP,pfm.LOG_SEVERITY_WARNING) end
	return success]]
end

util.load_baked_lightmap_uvs = function(lightmapCachePath, tEnts)
	log.msg("Loading baked lightmap UVs from '" .. lightmapCachePath .. "'...", pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP)
	local models = util.load_lightmap_uv_cache(lightmapCachePath)
	if models ~= false then
		log.msg(
			"Successfully loaded lightmap UV cache from '" .. lightmapCachePath .. "'!",
			pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP
		)
		return false
	end
	log.msg(
		"Failed to load lightmap UV cache from '" .. lightmapCachePath .. "'!",
		pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP,
		pfm.LOG_SEVERITY_WARNING
	)
	update_lightmap_data(tEnts)
	return true
end

util.bake_lightmaps = function(preview, lightIntensityFactor)
	include("/pfm/unirender.lua")
	if pfm.load_unirender() == false then
		log.msg(
			"Cannot bake lightmaps: Unable to load unirender library!",
			pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP,
			pfm.LOG_SEVERITY_WARNING
		)
		return false
	end
	local bakeSettings = ents.LightMapComponent.BakeSettings()
	if preview then
		bakeSettings.createAsRenderJob = false
		bakeSettings.width = 512
		bakeSettings.height = 512
		bakeSettings.samples = 200
		--[[bakeSettings.width = 1024
		bakeSettings.height = 1024
		bakeSettings.samples = 220]]
	else
		bakeSettings.createAsRenderJob = false
		bakeSettings.width = 4096
		bakeSettings.height = 4096
	end

	-- bakeSettings.samples = 120
	bakeSettings.exposure = 50.0
	bakeSettings:SetColorTransform("filmic-blender", "Medium Contrast")
	bakeSettings.rebuildUvAtlas = false
	bakeSettings.globalLightIntensityFactor = lightIntensityFactor or 1
	bakeSettings.skyStrength = 0.3
	-- unirender.PBRShader.set_global_albedo_override_color(Vector(0.8,0.8,0.8))
	local result = ents.LightMapComponent.bake_lightmaps(bakeSettings)
	if result then
		log.msg("Successfully created render job for scene lightmap atlas!", pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP)
	else
		log.msg(
			"Failed to create render job for scene lightmap atlas!",
			pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP,
			pfm.LOG_SEVERITY_WARNING
		)
	end
	-- unirender.PBRShader.set_global_albedo_override_color()
	return result
end

function util.bake_map_lightmaps(entLm, fileName)
	local lightSources = {}
	local lightmapReceivers = {}
	local lightmapInfluencers = {}
	-- TODO: Static props only?
	for ent, c in ents.citerator(ents.COMPONENT_MAP) do
		local includeEntityForLightmaps = false
		if ent:IsWorld() then
			includeEntityForLightmaps = true
		elseif ent:HasComponent(ents.COMPONENT_PROP) then
			includeEntityForLightmaps = true
		end
		if includeEntityForLightmaps then
			log.msg(
				"Including entity '" .. tostring(ent) .. "' for lightmap atlas!",
				pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP
			)
			table.insert(lightmapReceivers, ent)
		elseif ent:HasComponent(ents.COMPONENT_LIGHT) then
			log.msg(
				"Including light source '" .. tostring(ent) .. "' for lightmap!",
				pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP
			)
			table.insert(lightSources, ent)
		end
	end
	if #lightmapReceivers == 0 then
		log.msg(
			"Cannot bake lightmaps: No entities to bake lightmaps for!",
			pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP,
			pfm.LOG_SEVERITY_WARNING
		)
		return
	end
	if pfm.load_unirender() == false then
		log.msg(
			"Cannot bake lightmaps: Unable to load unirender library!",
			pfm.LOG_CATEGORY_LOG_CATEGORY_LIGHTMAP,
			pfm.LOG_SEVERITY_WARNING
		)
		return false
	end
	local path = util.Path.CreatePath(fileName)
	local uvCachePath = path + "lightmap_uv_cache"
	util.bake_lightmap_uvs(entLm, lightmapReceivers, uvCachePath:GetString())
	local lightMapPath = path + "lightmap_atlas"

	local entWorld = ents.get_world()
	local cCache = entWorld:AddComponent(ents.COMPONENT_LIGHT_MAP_DATA_CACHE)

	local width = 512
	local height = 512
	local sampleCount = 100
	local asJob = false
	local bakeCombined = false
	local job = pfm.bake.lightmaps(
		game.get_scene(),
		lightmapReceivers,
		lightmapInfluencers,
		lightSources,
		width,
		height,
		sampleCount,
		cCache:GetLightMapDataCache(),
		function(scene) end,
		bakeCombined,
		asJob
	)
	return true
end
