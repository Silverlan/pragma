--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/util/lightmap_bake.lua")
include("/pfm/bake/lightmaps.lua")

local Component = util.register_class("ents.LightmapBaker", BaseEntityComponent)

include("map.lua")

Component:RegisterMember("LightmapMode", udm.TYPE_UINT32, 1, {
	enumValues = {
		["NonDirectional"] = 0,
		["Directional"] = 1,
	},
})
-- Debug mode
Component:RegisterMember("LightmapUuid", udm.TYPE_STRING, "")
Component:RegisterMember("Width", udm.TYPE_UINT32, 2048)
Component:RegisterMember("Height", udm.TYPE_UINT32, 2048)
Component:RegisterMember("SampleCount", udm.TYPE_UINT32, 20000)
Component:RegisterMember("LightIntensityFactor", udm.TYPE_FLOAT, 1.0, {
	min = 0,
	max = 10,
})

function Component:Initialize()
	BaseEntityComponent.Initialize(self)
	self.m_baker = pfm.bake.LightmapBaker()

	self:AddEntityComponent(ents.COMPONENT_LIGHT_MAP_DATA_CACHE)
	self:AddEntityComponent("pfm_cuboid_bounds")
end
function Component:SetLightSources(lightSources)
	self.m_lightSources = lightSources
end
function Component:SetLightmapReceivers(lightmapReceivers)
	self.m_lightmapReceivers = lightmapReceivers
end
function Component:SetLightmapInfluencers(lightmapInfluencers)
	self.m_lightmapInfluencers = lightmapInfluencers
end
function Component:UpdateLightmapTargets()
	if self:BroadcastEvent(Component.EVENT_UPDATE_LIGHTMAP_TARGETS) == util.EVENT_REPLY_HANDLED then
		return
	end
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
	self:SetLightSources(lightSources)
	self:SetLightmapReceivers(lightmapReceivers)
	self:SetLightmapInfluencers(lightmapInfluencers)
end
function Component:OnRemove()
	self.m_baker:Clear()
end
function Component:OnEntitySpawn()
	self.m_lightmapUvCacheDirty = true
	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
	self:UpdateLightmapTargets()
end
function Component:UpdateLightmapData(tEnts)
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
function Component:SetLightmapUvCacheDirty()
	self.m_lightmapUvCacheDirty = true
	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
end
function Component:SetLightmapAtlasDirty()
	self.m_lightmapAtlasDirty = true
	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
end
function Component:GetLightmapJob()
	return self.m_lightmapJob
end
function Component:GetTargetLightmapUuid()
	local uuid = self:GetLightmapUuid()
	if #uuid > 0 then
		return uuid
	end
	return tostring(self:GetEntity():GetUuid())
end
function Component:ImportLightmapTexture(matIdentifier, texName, importTex)
	local ext = file.get_file_extension(importTex)
	if ext ~= nil and ext ~= "dds" and ext ~= "ktx" then
		local img = util.load_image(importTex, false, util.ImageBuffer.FORMAT_RGBA32)
		if img == nil then
			return
		end
		local newTexPath = file.remove_file_extension(importTex)
		if self:SaveLightmapImage(img, newTexPath) == false then
			return
		end
		return self:ImportLightmapTexture(matIdentifier, texName, newTexPath .. ".dds")
	end

	local lightmapC = self:GetEntity():AddComponent(ents.COMPONENT_LIGHT_MAP)
	if lightmapC == nil then
		return
	end
	local matName = lightmapC:GetMemberValue("lightmapMaterial")
	if #matName == 0 then
		local uuid = self:GetTargetLightmapUuid()
		local path = "projects/" .. uuid .. "/"
		file.create_path("materials/" .. path)
		matName = path .. "lightmap"
	end

	matName = asset.get_normalized_path(matName, asset.TYPE_MATERIAL)
	local mat = game.load_material(matName)
	if mat == nil then
		mat = game.create_material(matName, "lightmap")
	end

	local path = file.get_file_path(matName)
	local texPath = path .. texName
	texPath = file.remove_file_extension(texPath, { "dds" }) .. ".dds"

	local absPath = asset.get_asset_root_directory(asset.TYPE_MATERIAL) .. "/" .. texPath
	local res = file.copy(importTex, absPath)

	self:LogInfo(
		"Baked texture '" .. texPath .. "' imported as '" .. matIdentifier .. "' for material '" .. matName .. "'!"
	)
	asset.reload(texPath, asset.TYPE_TEXTURE)
	mat:SetTexture(matIdentifier, texPath)

	-- Update property
	local lightmapC = self:GetEntity():AddComponent(ents.COMPONENT_LIGHT_MAP)
	if lightmapC ~= nil then
		lightmapC:SetMemberValue("lightmapMaterial", mat:GetName())
		self:BroadcastEvent(Component.EVENT_ON_LIGHTMAP_MATERIAL_CHANGED, { mat:GetName() })
	end

	-- Save material
	mat:UpdateTextures()
	mat:InitializeShaderDescriptorSet()
	mat:SetLoaded(true)
	mat:Save()
	self:SetLightmapUvCacheDirty()
	self:SetLightmapAtlasDirty()
	return mat
end
function Component:SaveLightmapImage(img, texPath)
	local texInfo = util.TextureInfo()
	texInfo.inputFormat = util.TextureInfo.INPUT_FORMAT_R32G32B32A32_FLOAT
	texInfo.outputFormat = normalMap and util.TextureInfo.OUTPUT_FORMAT_BC3 or util.TextureInfo.OUTPUT_FORMAT_BC6
	texInfo.containerFormat = util.TextureInfo.CONTAINER_FORMAT_DDS
	texInfo.flags = bit.bor(texInfo.flags, util.TextureInfo.FLAG_BIT_GENERATE_MIPMAPS)

	img:Convert(util.ImageBuffer.FORMAT_RGBA32)
	return util.save_image(img, texPath .. ".dds", texInfo)
end
function Component:SaveLightmapTexture(jobResult, resultIdentifier, matIdentifier, texName, normalMap)
	local lightmapC = self:GetEntity():AddComponent(ents.COMPONENT_LIGHT_MAP)
	if lightmapC == nil then
		return
	end
	local matName = lightmapC:GetMemberValue("lightmapMaterial")
	if #matName == 0 then
		local uuid = self:GetTargetLightmapUuid()
		local path = "projects/" .. uuid .. "/"
		file.create_path("materials/" .. path)
		matName = path .. "lightmap"
	end

	matName = asset.get_normalized_path(matName, asset.TYPE_MATERIAL)
	local mat = game.load_material(matName)
	if mat == nil then
		mat = game.create_material(matName, "lightmap")
	end

	local img
	if type(resultIdentifier) == "string" then
		img = jobResult:GetImage(resultIdentifier)
	else
		img = resultIdentifier
	end
	if img == nil then
		self:LogInfo("Baked texture '" .. matIdentifier .. "' not found, ignoring...")
		return
	end

	local path = file.get_file_path(matName)
	local texPath = path .. texName
	local r = self:SaveLightmapImage(img, asset.get_asset_root_directory(asset.TYPE_MATERIAL) .. "/" .. texPath)
	if r then
		self:LogInfo("Baked texture '" .. matIdentifier .. "' saved as '" .. texPath .. "'!")
		asset.reload(texPath, asset.TYPE_TEXTURE)
		mat:SetTexture(matIdentifier, texPath)
	else
		self:LogWarn("Failed to save baked texture '" .. matIdentifier .. "' as '" .. texPath .. "'!")
	end
	return mat
end
function Component:OnTick(dt)
	if self.m_lightmapJob ~= nil then
		self.m_lightmapJob:Poll()
		if self.m_lightmapJob:IsComplete() then
			if self.m_lightmapJob:IsSuccessful() then
				local result = self.m_lightmapJob:GetResult()
				local mat = self:SaveLightmapTexture(result, "Diffuse", "diffuse_map", "lightmap_diffuse")
				if mat == nil then
					mat = self:SaveLightmapTexture(
						result,
						"DiffuseDirect",
						"diffuse_direct_map",
						"lightmap_diffuse_direct"
					)
					self:SaveLightmapTexture(
						result,
						"DiffuseIndirect",
						"diffuse_indirect_map",
						"lightmap_diffuse_indirect"
					)
					if mat ~= nil then
						mat:GetDataBlock():RemoveValue("diffuse_map")
					end
				else
					mat:GetDataBlock():RemoveValue("diffuse_direct_map")
					mat:GetDataBlock():RemoveValue("diffuse_indirect_map")
				end

				if mat ~= nil then
					mat:UpdateTextures()
					mat:InitializeShaderDescriptorSet()
					mat:SetLoaded(true)
					if mat:Save() == false then
						self:LogWarn("Failed to save lightmap atlas material as '" .. mat:GetName() .. "'!")
					else
						local lightmapC = self:GetEntity():AddComponent(ents.COMPONENT_LIGHT_MAP)
						if lightmapC ~= nil then
							lightmapC:SetMemberValue("lightmapMaterial", mat:GetName())
							self:BroadcastEvent(Component.EVENT_ON_LIGHTMAP_MATERIAL_CHANGED, { mat:GetName() })
						end
					end
				end
			else
				print("Lightmap baking error: ", self.m_lightmapJob:GetResultMessage())
			end
			self.m_lightmapJob = nil
		end
	end
	if self.m_dirLightmapJob ~= nil then
		self.m_dirLightmapJob:Poll()
		if self.m_dirLightmapJob:IsComplete() then
			if self.m_dirLightmapJob:IsSuccessful() then
				local image = self.m_dirLightmapJob:GetResult()
				local mat = self:SaveLightmapTexture(image, image, "dominant_direction_map", "lightmap_normal", true)
				if mat ~= nil then
					mat:UpdateTextures()
					mat:InitializeShaderDescriptorSet()
					mat:SetLoaded(true)
					if mat:Save() == false then
						self:LogWarn("Failed to save lightmap atlas material as '" .. mat:GetName() .. "'!")
					else
						local lightmapC = self:GetEntity():AddComponent(ents.COMPONENT_LIGHT_MAP)
						if lightmapC ~= nil then
							lightmapC:SetMemberValue("lightmapMaterial", mat:GetName())
							self:BroadcastEvent(Component.EVENT_ON_LIGHTMAP_MATERIAL_CHANGED, { mat:GetName() })
						end
					end
				end
			else
				print("Lightmap baking error: ", self.m_dirLightmapJob:GetResultMessage())
			end
			self.m_dirLightmapJob = nil
		end
	end
	if self.m_lightmapUvCacheDirty then
		self.m_lightmapUvCacheDirty = nil
		self:ReloadLightmapDataCache()
	end
	if self.m_lightmapAtlasDirty then
		self.m_lightmapAtlasDirty = nil
		self:UpdateLightmapAtlas()
	end
	pfm.tag_render_scene_as_dirty()

	if self.m_lightmapJob == nil and self.m_dirLightmapJob == nil then
		if self.m_bakeQueue ~= nil then
			local item = self.m_bakeQueue[1]
			table.remove(self.m_bakeQueue, 1)
			if item ~= nil then
				if item.type == "directional" then
					self:GenerateDirectionalLightmaps()
				elseif item.type == "diffuse" then
					self:GenerateLightmaps()
				end
				return
			end
		end
		self:SetTickPolicy(ents.TICK_POLICY_NEVER)
		self:OnBakingCompleted()
		self:BroadcastEvent(Component.EVENT_ON_BAKING_COMPLETED)
	end
end
function Component:UpdateLightmapAtlas()
	local lightmapC = self:GetEntity():AddComponent(ents.COMPONENT_LIGHT_MAP)
	if lightmapC ~= nil then
		--[[local tex = asset.reload(self:GetLightmapAtlas(),asset.TYPE_TEXTURE)
		tex = (tex ~= nil) and tex:GetVkTexture() or nil
		if(tex ~= nil) then lightmapC:SetLightmapAtlas(tex) end

		local texDir = asset.reload(self:GetDirectionalLightmapAtlas(),asset.TYPE_TEXTURE)
		texDir = (texDir ~= nil) and texDir:GetVkTexture() or nil
		if(texDir ~= nil) then lightmapC:SetDirectionalLightmapAtlas(texDir) end]]

		lightmapC:ReloadLightmapData()
	end
end

function Component:IsLightmapUvRebuildRequired()
	local lmC = self:GetEntityComponent(ents.COMPONENT_LIGHT_MAP)
	if lmC == nil then
		return true
	end

	local c = self:GetEntity():GetComponent("light_map_data_cache")
	if c == nil then
		return true
	end
	local cachePath = c:GetMemberValue("lightmapDataCache")
	local cache, err = ents.LightMapComponent.DataCache.load(cachePath)
	if cache == false then
		return true
	end
	if cache:GetLightmapEntity() ~= lmC:GetEntity():GetUuid() then
		return true
	end

	local instances = cache:GetInstanceIds()
	local bakeEntities = self.m_lightmapReceivers
	local curUuids = {}
	for _, ent in ipairs(bakeEntities) do
		local uuid = ent:GetUuid()
		curUuids[uuid] = true
		local pose = ent:GetPose()
		local cachePose = cache:GetInstancePose(uuid)
		if cachePose ~= nil and cachePose ~= pose then
			return true
		end
	end

	for _, uuid in ipairs(cache:GetInstanceIds()) do
		if curUuids[uuid] ~= true then
			return true
		end
	end
	return false
end

function Component:GenerateLightmapUvs()
	local lmC = self:GetEntityComponent(ents.COMPONENT_LIGHT_MAP)
	if lmC == nil then
		self:LogWarn("Failed to generate lightmap uvs: No light map component!")
		return false
	end

	local uuid = self:GetTargetLightmapUuid()
	local path = "data/projects/" .. uuid .. "/"
	file.create_path(path)
	local cachePath = path .. "lightmap_data_cache"

	local meshFilter
	local minArea = Vector()
	local maxArea = Vector()
	local cuboidC = self:GetEntityComponent(ents.COMPONENT_PFM_CUBOID_BOUNDS)
	if cuboidC ~= nil then
		minArea, maxArea = cuboidC:GetBounds()
	end
	local l = minArea:DistanceSqr(maxArea)
	if l > 0.0001 then
		meshFilter = function(ent, mesh, subMesh)
			local min, max = subMesh:GetBounds()
			local pose = ent:GetPose()
			min, max = boundingvolume.get_rotated_aabb(min, max, pose:GetRotation())
			min = min + pose:GetOrigin()
			max = max + pose:GetOrigin()
			local res = intersect.aabb_with_aabb(min, max, minArea, maxArea)
			return res ~= intersect.RESULT_OUTSIDE
		end
	end
	if
		self.m_baker:BakeUvs(lmC:GetEntity(), self.m_lightmapReceivers, util.get_addon_path() .. cachePath, meshFilter)
		== false
	then
		self:LogWarn("Failed to bake lightmap uvs!")
		return false
	end

	local cCache = self:GetEntityComponent(ents.COMPONENT_LIGHT_MAP_DATA_CACHE)
	if cCache == nil then
		self:LogWarn("Failed to generate lightmap uvs: No lightmap data cache component!")
		return false
	end

	cCache:SetMemberValue("lightmapDataCache", cachePath)
	self:BroadcastEvent(Component.EVENT_ON_LIGHTMAP_DATA_CACHE_CHANGED, { cachePath })
	self:ReloadLightmapDataCache()
	return true
end

function Component:ReloadLightmapDataCache()
	local cCache = self:GetEntityComponent(ents.COMPONENT_LIGHT_MAP_DATA_CACHE)
	if cCache == nil then
		return
	end
	cCache:ReloadCache()
end

function Component:GenerateDirectionalLightmaps(preview, lightIntensityFactor)
	local cCache = self:GetEntityComponent(ents.COMPONENT_LIGHT_MAP_DATA_CACHE)
	if cCache == nil then
		return
	end
	local cache = cCache:GetLightMapDataCache()
	if cache == nil then
		return
	end
	local width = self:GetWidth()
	local height = self:GetHeight()
	if width == nil or height == nil then
		return
	end
	local bakeEntities = self.m_lightmapReceivers
	local lightSources = {}
	for _, ent in ipairs(self.m_lightSources) do
		table.insert(lightSources, ent:GetComponent(ents.COMPONENT_LIGHT))
	end
	local job = pfm.bake.directional_lightmaps(bakeEntities, lightSources, width, height, cache)
	job:Start()
	self.m_dirLightmapJob = job
	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
	return job
end

function Component:GenerateLightmaps(preview, lightIntensityFactor, asJob)
	local cCache = self:GetEntityComponent(ents.COMPONENT_LIGHT_MAP_DATA_CACHE)
	if cCache == nil then
		return false
	end
	if preview == nil then
		preview = true
	end
	-- util.bake_lightmaps(preview,lightIntensityFactor)
	local width = self:GetWidth()
	local height = self:GetHeight()
	if width == nil or height == nil then
		return false
	end

	-- TODO

	local mode = self:GetLightmapMode()
	local bakeCombined = (mode == Component.LIGHTMAP_MODE_NON_DIRECTIONAL)
	self.m_baker:Start(
		self.m_lightmapReceivers,
		self.m_lightmapInfluencers,
		self.m_lightSources,
		width,
		height,
		self:GetSampleCount(),
		cCache:GetLightMapDataCache(),
		function(scene)
			scene:SetLightIntensityFactor(self:GetLightIntensityFactor())
		end,
		bakeCombined,
		asJob
	)

	if asJob then
		return true
	end
	self.m_lightmapJob = self.m_baker.m_job

	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
	return self:GetLightmapJob()
end
function Component:ReloadLightmapData(tEnts)
	--[[local entWorld = tEnts[1]
	local lightmapC = entWorld:AddComponent(ents.COMPONENT_LIGHT_MAP)
	if(lightmapC ~= nil) then
		local tex = asset.load(lightmap,asset.TYPE_TEXTURE)
		tex = (tex ~= nil) and tex:GetVkTexture() or nil
		if(tex ~= nil) then lightmapC:SetLightmapAtlas(tex) end
		lightmapC:ReloadLightmapData()
	end
	vrp.load_baked_lightmap_uvs("lm_cache",tEnts)]]
end
ents.COMPONENT_LIGHTMAP_BAKER = ents.register_component("lightmap_baker", Component)
Component.EVENT_UPDATE_LIGHTMAP_TARGETS =
	ents.register_component_event(ents.COMPONENT_LIGHTMAP_BAKER, "update_lightmap_targets")
Component.EVENT_ON_BAKING_COMPLETED =
	ents.register_component_event(ents.COMPONENT_LIGHTMAP_BAKER, "on_baking_completed")
Component.EVENT_ON_LIGHTMAP_MATERIAL_CHANGED =
	ents.register_component_event(ents.COMPONENT_LIGHTMAP_BAKER, "on_lightmap_material_changed")
Component.EVENT_ON_LIGHTMAP_DATA_CACHE_CHANGED =
	ents.register_component_event(ents.COMPONENT_LIGHTMAP_BAKER, "on_lightmap_data_cache_changed")
