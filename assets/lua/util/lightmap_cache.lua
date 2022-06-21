--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local LIGHTMAP_CACHE_VERSION = 1

function util.load_lightmap_uv_cache(fileName)
	fileName = file.remove_file_extension(fileName) .. ".lmd_b"

	local udmData,err = udm.create("PLMD",LIGHTMAP_CACHE_VERSION)
	if(udmData == false) then
		console.print_warning(err)
		return false
	end

	local udmData,err = udm.load(fileName)
	if(udmData == false) then
		console.print_warning(err)
		return false
	end

	local lightmapData = {}
	local assetData = udmData:GetAssetData():GetData()
	local udmEntities = assetData:Get("entities")
	for udmEntity in udmEntities:It() do
		local pose = udmEntity:GetValue("pose")
		local model = udmEntity:GetValue("model")

		local data = udmEntity:Get("data")
		local err
		if(data:IsValid()) then
			local mdl
			mdl,err = game.Model.Load(udm.AssetData(udmEntity:Get("data"),"PMDL",1))
			if(mdl ~= false) then
				lightmapData[model] = {
					pose = pose,
					meshData = meshData,
					model = mdl
				}
			end
		else
			err = "Invalid UDM model data"
		end
		if(err ~= nil) then console.print_warning("Failed to load lightmapped model '" .. (model or "unknown") .. "': " .. err) end
	end

	local tEnts = {}
	--[[for ent in ents.iterator({ents.IteratorFilterComponent(ents.COMPONENT_MODEL),ents.IteratorFilterComponent(ents.COMPONENT_RENDER),ents.IteratorFilterComponent(ents.COMPONENT_TRANSFORM)}) do
		local mdl = ent:GetModel()
		if(mdl ~= nil) then
			local mdlName = asset.get_normalized_path(mdl:GetName(),asset.TYPE_MODEL)
			local lmData = lightmapData[mdlName]
			if(lmData ~= nil) then
				local pose = ent:GetPose()
				if(pose:GetOrigin():DistanceSqr(lmData.pose:GetOrigin()) < 0.001 and math.abs(pose:GetRotation():Distance(lmData.pose:GetRotation())) < 0.001) then
					table.insert(tEnts,ent)
					ent:SetModel(lmData.model)
				end
			end
		end
	end]]
	return tEnts
end

function util.save_lightmap_uv_cache(fileName,entities)
	file.create_path(file.get_file_path(fileName))
	fileName = file.remove_file_extension(fileName) .. ".lmd_b"

	local udmData,err = udm.create("PLMD",LIGHTMAP_CACHE_VERSION)
	if(udmData == false) then
		console.print_warning(err)
		return
	end

	local assetData = udmData:GetAssetData():GetData()
	local udmEntities = assetData:AddArray("entities",#entities)
	local entIdx = 0
	for _,ent in ipairs(entities) do
		local mdl = ent:GetModel()
		local meshGroups = mdl:GetMeshGroups()
		local hasLightmappedMeshes = false
		for i,meshGroup in ipairs(meshGroups) do
			local meshes = meshGroup:GetMeshes()
			for j,mesh in ipairs(meshes) do
				local subMeshes = mesh:GetSubMeshes()
				for k,subMesh in ipairs(subMeshes) do
					local hasLightmapSet = subMesh:HasUVSet("lightmap")
					if(hasLightmapSet) then
						hasLightmappedMeshes = true
						break
					end
				end
				if(hasLightmappedMeshes) then break end
			end
			if(hasLightmappedMeshes) then break end
		end

		if(hasLightmappedMeshes) then
			local pose = ent:GetPose()
			local model = asset.get_normalized_path(ent:GetModel():GetName(),asset.TYPE_MODEL)

			-- We'll identify lightmapped entities by their position and model
			local origin = pose:GetOrigin()
			local udmEntity = udmEntities:Get(entIdx)
			entIdx = entIdx +1
			udmEntity:Set("pose",math.Transform(pose:GetOrigin(),pose:GetRotation()))
			udmEntity:Set("model",model)

			ent:GetModel():Save(udm.AssetData(udmEntity:Get("data"),"PMDL",1))
		end
	end
	udmEntities:Resize(entIdx)

	return udmData:Save(fileName)
end
