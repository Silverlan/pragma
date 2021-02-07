--[[
    Copyright (C) 2019  Florian Weischer

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local LIGHTMAP_CACHE_VERSION = 1

function util.load_lightmap_uv_cache(fileName)
	fileName = file.remove_file_extension(fileName) .. ".lmc"
	local f = file.open(fileName,bit.bor(file.OPEN_MODE_READ,file.OPEN_MODE_BINARY))
	if(f == nil) then return false end
	local size = f:Size()
	local ds = f:Read(size)
	f:Close()

	local header = ds:ReadString(5)
	if(header ~= "PRLMC") then return false end
	local version = ds:ReadUInt32()
	if(version < 1 or version > LIGHTMAP_CACHE_VERSION) then return false end

	local dictionary = {}
	local numModels = ds:ReadUInt32()
	for i=1,numModels do
		local mdlName = ds:ReadString()
		local dataOffset = ds:ReadUInt64()
		dictionary[i] = {mdlName,dataOffset}
	end
	local tModels = {}
	for i,data in ipairs(dictionary) do
		local mdl = game.load_model(data[1])
		if(mdl ~= nil) then
			table.insert(tModels,mdl)
			ds:Seek(data[2])
			local numGroups = ds:ReadUInt32()
			for j=1,numGroups do
				local meshGroup = (mdl ~= nil) and mdl:GetMeshGroup(j -1) or nil
				local numMeshes = ds:ReadUInt32()
				for k=1,numMeshes do
					local mesh = (meshGroup ~= nil) and meshGroup:GetMesh(k -1) or nil
					local numSubMeshes = ds:ReadUInt32()
					for l=1,numSubMeshes do
						local subMesh = (mesh ~= nil) and mesh:GetSubMesh(l -1) or nil
						local hasLightmapSet = ds:ReadBool()
						if(hasLightmapSet) then
							subMesh:AddUVSet("lightmap")
							local numVerts = ds:ReadUInt32()
							subMesh:SetVertexCount(numVerts)
							for m=1,numVerts do
								local pos = ds:ReadVector()
								local uv = ds:ReadVector2()
								local normal = ds:ReadVector()
								local tangent = ds:ReadVector4()
								local uvLightmap = ds:ReadVector2()

								subMesh:SetVertexPosition(m -1,pos)
								subMesh:SetVertexNormal(m -1,normal)
								subMesh:SetVertexUV(m -1,uv)
								subMesh:SetVertexTangent(m -1,tangent)

								subMesh:SetVertexUV("lightmap",m -1,uvLightmap)
							end

							subMesh:ClearTriangles()
							local numIndices = ds:ReadUInt32()
							for i=1,numIndices,3 do
								local idx0 = ds:ReadUInt32()
								local idx1 = ds:ReadUInt32()
								local idx2 = ds:ReadUInt32()
								subMesh:AddTriangle(idx0,idx1,idx2)
							end
						end
						subMesh:Update(game.Model.FUPDATE_ALL)
					end
				end
			end
		end
	end
	return tModels
end

function util.save_lightmap_uv_cache(fileName,models)
	fileName = file.remove_file_extension(fileName) .. ".lmc"
	local ds = util.DataStream()
	ds:WriteString("PRLMC",false)
	ds:WriteUInt32(LIGHTMAP_CACHE_VERSION)
	ds:WriteUInt32(#models)
	local tOffsets = {}
	for i,mdl in ipairs(models) do
		ds:WriteString(mdl:GetName())
		tOffsets[i] = ds:Tell()
		ds:WriteUInt64(0)
	end
	for i,mdl in ipairs(models) do
		local offset = ds:Tell()
		ds:Seek(tOffsets[i])
		ds:WriteUInt64(offset)
		ds:Seek(offset)

		local meshGroups = mdl:GetMeshGroups()
		ds:WriteUInt32(#meshGroups)
		for _,meshGroup in ipairs(meshGroups) do
			local meshes = meshGroup:GetMeshes()
			ds:WriteUInt32(#meshes)
			for _,mesh in ipairs(meshes) do
				local subMeshes = mesh:GetSubMeshes()
				ds:WriteUInt32(#subMeshes)
				for _,subMesh in ipairs(subMeshes) do
					local hasLightmapSet = subMesh:HasUVSet("lightmap")
					ds:WriteBool(hasLightmapSet)
					if(hasLightmapSet) then
						local numVerts = subMesh:GetVertexCount()
						ds:WriteUInt32(numVerts)
						for i=1,numVerts do
							local v = subMesh:GetVertex(i -1)
							ds:WriteVector(v.position)
							ds:WriteVector2(v.uv)
							ds:WriteVector(v.normal)
							ds:WriteVector4(v.tangent)
							ds:WriteVector2(subMesh:GetVertexUV("lightmap",i -1))
						end

						local indices = subMesh:GetTriangles()
						ds:WriteUInt32(#indices)
						for i=1,#indices do
							ds:WriteUInt32(indices[i])
						end
					end
				end
			end
		end
	end
	ds:Seek(0)
	local f = file.open(fileName,bit.bor(file.OPEN_MODE_WRITE,file.OPEN_MODE_BINARY))
	if(f == nil) then return false end
	f:Write(ds)
	f:Close()
	return true
end
