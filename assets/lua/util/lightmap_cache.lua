--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local LIGHTMAP_CACHE_VERSION = 1

function util.load_lightmap_uv_cache(fileName)
	fileName = file.remove_file_extension(fileName) .. ".lmdb"

	local udmData,err = udm.create("PLMD",LIGHTMAP_CACHE_VERSION)
	if(udmData == false) then
		console.print_warning(err)
		return
	end

	local udmData,err = udm.load(fileName,LIGHTMAP_CACHE_VERSION)
	if(udmData == false) then
		console.print_warning(err)
		return
	end

	local lightmapData = {}
	local assetData = udmData:GetAssetData()
	local udmEntities = assetData:Get("entities")
	for udmEntity in udmEntities:It() do
		local pose = udmEntity:GetValue("pose")
		local model = udmEntity:GetValue("model")
		local udmMeshes = udmEntity:Get("meshes")
		local meshData = {}
		for udmMesh in udmMeshes:It() do
			local meshGroup = udmMesh:GetValue("meshGroup")
			local mesh = udmMesh:GetValue("mesh")
			local subMesh = udmMesh:GetValue("subMesh")
			local dsVertexData = udmMesh:Get("vertexData"):GetBlobData()
			local dsIndexData = udmMesh:Get("indexData"):GetBlobData()
			local dsLightmapUvData = udmMesh:Get("lightmapUvData"):GetBlobData()
			table.insert(meshData,{
				meshGroup = meshGroup,
				mesh = mesh,
				subMesh = subMesh,
				dsVertexData = dsVertexData,
				dsIndexData = dsIndexData,
				dsLightmapUvData = dsLightmapUvData
			})
		end
		lightmapData[model] = {
			pose = pose,
			meshData = meshData
		}
	end

	local tEnts = {}
	for ent in ents.iterator({ents.IteratorFilterComponent(ents.COMPONENT_MODEL),ents.IteratorFilterComponent(ents.COMPONENT_RENDER),ents.IteratorFilterComponent(ents.COMPONENT_TRANSFORM)}) do
		local mdl = ent:GetModel()
		if(mdl ~= nil) then
			local mdlName = asset.get_normalized_path(mdl:GetName(),asset.TYPE_MODEL)
			local lmData = lightmapData[mdlName]
			if(lmData ~= nil) then
				local pose = ent:GetPose()
				if(pose:GetOrigin():DistanceSqr(lmData.pose:GetOrigin()) < 0.001 and math.abs(pose:GetRotation():Distance(lmData.pose:GetRotation())) < 0.001) then
					table.insert(tEnts,ent)
					for _,meshData in ipairs(lmData.meshData) do
						local meshGroup = mdl:GetMeshGroup(meshData.meshGroup)
						local mesh = (meshGroup ~= nil) and meshGroup:GetMesh(meshData.mesh) or nil
						local subMesh = (mesh ~= nil) and mesh:GetSubMesh(meshData.subMesh) or nil
						if(subMesh ~= nil) then
							subMesh:AddUVSet("lightmap")

							local dsVertexData = meshData.dsVertexData
							local dsIndexData = meshData.dsIndexData
							local dsLightmapUvData = meshData.dsLightmapUvData
							local numVerts = dsVertexData:GetSize() /util.SIZEOF_VERTEX
							local numIndices = dsIndexData:GetSize() /util.SIZEOF_SHORT

							subMesh:SetVertexCount(numVerts)
							for i=1,numVerts do
								subMesh:SetVertex(i -1,dsVertexData:ReadVertex())
								subMesh:SetVertexUV("lightmap",i -1,dsLightmapUvData:ReadVector2())
							end

							subMesh:ClearIndices()
							for i=1,numIndices,3 do
								local idx0 = dsIndexData:ReadUInt16()
								local idx1 = dsIndexData:ReadUInt16()
								local idx2 = dsIndexData:ReadUInt16()
								subMesh:AddTriangle(idx0,idx1,idx2)
							end
						end
						subMesh:Update(game.Model.FUPDATE_ALL)
					end
				end
			end
		end
	end
	return tEnts
end

function util.save_lightmap_uv_cache(fileName,entities)
	file.create_path(file.get_file_path(fileName))
	fileName = file.remove_file_extension(fileName) .. ".lmdb"

	local udmData,err = udm.create("PLMD",LIGHTMAP_CACHE_VERSION)
	if(udmData == false) then
		console.print_warning(err)
		return
	end

	local assetData = udmData:GetAssetData()
	local udmEntities = assetData:AddArray("entities",#entities)
	local entIdx = 0
	for _,ent in ipairs(entities) do
		local mdl = ent:GetModel()
		local meshGroups = mdl:GetMeshGroups()
		local lightmappedMeshes = {}
		for i,meshGroup in ipairs(meshGroups) do
			local meshes = meshGroup:GetMeshes()
			for j,mesh in ipairs(meshes) do
				local subMeshes = mesh:GetSubMeshes()
				for k,subMesh in ipairs(subMeshes) do
					local hasLightmapSet = subMesh:HasUVSet("lightmap")
					if(hasLightmapSet) then
						local numVerts = subMesh:GetVertexCount()
						local dsVertexData = util.DataStream(numVerts *util.SIZEOF_VERTEX)
						for i=0,numVerts -1 do dsVertexData:WriteVertex(subMesh:GetVertex(i)) end

						local indices = subMesh:GetIndices()
						local numIndices = #indices
						local dsIndexData = util.DataStream(numVerts *util.SIZEOF_SHORT)
						for _,idx in ipairs(indices) do dsIndexData:WriteUInt16(idx) end

						local dsLightmapUvs = util.DataStream(numVerts *util.SIZEOF_VECTOR2)
						for i=0,numVerts -1 do dsLightmapUvs:WriteVector2(subMesh:GetVertexUV("lightmap",i)) end
						
						table.insert(lightmappedMeshes,{
							meshGroup = i -1,
							mesh = j -1,
							subMesh = k -1,
							vertexData = dsVertexData,
							indexData = dsIndexData,
							lightmapUvData = dsLightmapUvs
						})
					end
				end
			end
		end

		if(#lightmappedMeshes > 0) then
			local pose = ent:GetPose()
			local model = asset.get_normalized_path(ent:GetModel():GetName(),asset.TYPE_MODEL)

			-- We'll identify lightmapped entities by their position and model
			local origin = pose:GetOrigin()
			local udmEntity = udmEntities:Get(entIdx)
			entIdx = entIdx +1
			udmEntity:Set("pose",math.Transform(pose:GetOrigin(),pose:GetRotation()))
			udmEntity:Set("model",model)

			local udmMeshes = udmEntity:AddArray("meshes",#lightmappedMeshes)
			for i,meshInfo in ipairs(lightmappedMeshes) do
				local udmMesh = udmMeshes:Get(i -1)
				udmMesh:SetValue("meshGroup",udm.TYPE_UINT32,meshInfo.meshGroup)
				udmMesh:SetValue("mesh",udm.TYPE_UINT32,meshInfo.mesh)
				udmMesh:SetValue("subMesh",udm.TYPE_UINT32,meshInfo.subMesh)

				udmMesh:SetValue("vertexData",udm.TYPE_BLOB_LZ4,udm.compress_lz4(meshInfo.vertexData))
				udmMesh:SetValue("indexData",udm.TYPE_BLOB_LZ4,udm.compress_lz4(meshInfo.indexData))
				udmMesh:SetValue("lightmapUvData",udm.TYPE_BLOB_LZ4,udm.compress_lz4(meshInfo.lightmapUvData))
			end
		end
	end
	udmEntities:Resize(entIdx)

	udmData:Save(fileName)
end
