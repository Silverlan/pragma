--[[
    Copyright (C) 2019  Florian Weischer

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("util.UVAtlasGenerator")
function util.UVAtlasGenerator:__init(lmUuid)
	local r = engine.load_library("pr_xatlas")
	if(r ~= true) then
		console.print_warning("Unable to load openvr module: " .. r)
		return
	end
	self.m_atlas = xatlas.create()
	self.m_entities = {}
	self.m_lightmapEntityUuid = lmUuid
	self.m_numInputMeshes = 0
end

function util.UVAtlasGenerator:SetCuboidBounds(min,max)
	self.m_bounds = {min,max}
end

function util.UVAtlasGenerator:AddEntity(ent,meshFilter)
	local mdl = ent:GetModel()
	local mdlC = ent:GetComponent(ents.COMPONENT_MODEL)
	local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
	if(mdl == nil or mdlC == nil or renderC == nil) then return end
	local name = mdl:GetName()
	local origMdl = game.load_model(name)
	if(origMdl == nil) then return end
	mdl = origMdl
	if(self.m_entities[ent] ~= nil) then return end
	mdl = mdl:Copy(bit.bor(game.Model.FCOPY_DEEP,game.Model.FCOPY_BIT_COPY_UNIQUE_IDS))

	local skin = mdlC:GetSkin()
	local bodyGroups = mdlC:GetBodyGroups()
	ent:SetModel(mdl)
	mdlC:SetSkin(skin)
	mdlC:SetBodyGroups(bodyGroups)

	local lightMapReceiverC = ent:GetComponent(ents.COMPONENT_LIGHT_MAP_RECEIVER)
	local deleteGeometryOutOfBounds = (lightMapReceiverC ~= nil) and lightMapReceiverC:ShouldRemoveOutOfBoundsGeometry() or false
	self.m_entities[ent] = {}
	local origMdlSubMeshes = {}
	for _,mg in ipairs(mdl:GetMeshGroups()) do
		for _,m in ipairs(mg:GetMeshes()) do
			for _,sm in ipairs(m:GetSubMeshes()) do
				origMdlSubMeshes[tostring(sm:GetUuid())] = sm
			end
		end
	end
	local renderMeshes = renderC:GetLODMeshes()
	local entPose = ent:GetPose()
	for _,mesh in ipairs(renderMeshes) do
		for _,renderSubMesh in ipairs(mesh:GetSubMeshes()) do
			local subMesh = origMdlSubMeshes[tostring(renderSubMesh:GetUuid())]
			if(subMesh ~= nil) then
			-- if(subMesh:HasUVSet("lightmap")) then
				local mat = mdl:GetMaterial(subMesh:GetSkinTextureIndex()) -- mdlC:GetRenderMaterial(subMesh:GetSkinTextureIndex())
				if(mat ~= nil) then
					if(meshFilter ~= nil and meshFilter(mesh,subMesh) == false) then
						if(deleteGeometryOutOfBounds) then
							local newMesh = subMesh:Copy(true)
							newMesh:ClearVertices()
							newMesh:ClearVertexWeights()
							newMesh:ClearIndices()
							newMesh:Update(game.Model.FUPDATE_ALL_DATA)
							table.insert(self.m_entities[ent],{
								origMesh = subMesh
							})
						end
					else
						local lmMesh = subMesh
						local exMesh
						if(self.m_bounds ~= nil) then
							local min,max = self.m_bounds[1],self.m_bounds[2]
							local indices = subMesh:GetIndices()
							local verts = subMesh:GetVertices()
							local lmMeshData = {
								newVerts = {},
								newIndices = {},
								oldIndexToNewIndex = {}
							}
							local exMeshData = {
								newVerts = {},
								newIndices = {},
								oldIndexToNewIndex = {}
							}
							local function add_vertex(meshData,idx)
								if(meshData.oldIndexToNewIndex[idx] ~= nil) then
									table.insert(meshData.newIndices,meshData.oldIndexToNewIndex[idx])
									return meshData.oldIndexToNewIndex[idx]
								end
								local newIdx = #meshData.newVerts
								table.insert(meshData.newVerts,subMesh:GetVertex(idx))
								table.insert(meshData.newIndices,newIdx)
								meshData.oldIndexToNewIndex[idx] = newIdx
								return newIdx
							end
							for i=1,#indices,3 do
								local idx0 = indices[i]
								local idx1 = indices[i +1]
								local idx2 = indices[i +2]
								local v0 = verts[idx0 +1]
								local v1 = verts[idx1 +1]
								local v2 = verts[idx2 +1]
								local v0g = entPose *v0
								local v1g = entPose *v1
								local v2g = entPose *v2
								local hasIntersect = intersect.aabb_with_triangle(min,max,v0g,v1g,v2g)
								if(hasIntersect == true) then
									add_vertex(lmMeshData,idx0)
									add_vertex(lmMeshData,idx1)
									add_vertex(lmMeshData,idx2)
								else
									add_vertex(exMeshData,idx0)
									add_vertex(exMeshData,idx1)
									add_vertex(exMeshData,idx2)
								end
							end

							local function buildMesh(meshData)
								local newMesh = subMesh:Copy(true)
								newMesh:SetUuid(subMesh:GetUuid())
								newMesh:ClearVertices()
								newMesh:ClearVertexWeights()
								newMesh:ClearIndices()
								for _,v in ipairs(meshData.newVerts) do
									newMesh:AddVertex(v)
								end
								for i=1,#meshData.newIndices,3 do
									local idx0 = meshData.newIndices[i]
									local idx1 = meshData.newIndices[i +1]
									local idx2 = meshData.newIndices[i +2]
									newMesh:AddTriangle(idx0,idx1,idx2)
								end
								newMesh:Update(game.Model.FUPDATE_ALL_DATA)
								return newMesh
							end
							lmMesh = buildMesh(lmMeshData)
							if(deleteGeometryOutOfBounds ~= true) then exMesh = buildMesh(exMeshData) end
						end

						self.m_atlas:AddMesh(lmMesh,mat)
						self.m_numInputMeshes = self.m_numInputMeshes +1
						table.insert(self.m_entities[ent],{
							origMesh = subMesh or lmMesh,
							subMesh = lmMesh,
							exMesh = exMesh,
							xatlasMeshIndex = self.m_numInputMeshes
						})
					end
				end
			-- end
			end
		end
	end
end

function util.UVAtlasGenerator:Generate(lightmapCachePath)
	local meshes = self.m_atlas:Generate()
	if(#meshes ~= self.m_numInputMeshes) then
		error("Number of output meshes doesn't match number of input meshes!")
		return
	end

	for ent,entMeshes in pairs(self.m_entities) do
		if(ent:IsValid()) then
			for _,meshInfo in ipairs(entMeshes) do
				local mesh = meshInfo.origMesh
				local atlasMesh = meshes[meshInfo.xatlasMeshIndex]
				local origIndexCount = mesh:GetIndexCount()
				local newIndexCount = (atlasMesh ~= nil) and atlasMesh:GetIndexCount() or nil
				meshInfo.restructuredMesh = true -- (newIndexCount ~= origIndexCount)
			end
		end
	end

	-- Clear buffers (to free up memory)
	for ent,meshes in pairs(self.m_entities) do
		if(ent:IsValid()) then
			for _,meshInfo in ipairs(meshes) do
				if(meshInfo.restructuredMesh == true) then
					local mesh = meshInfo.origMesh
					local sceneMesh = mesh:GetSceneMesh()
					if(sceneMesh ~= nil) then sceneMesh:ClearBuffers() end
				end
			end
		end
	end

	local models = {}
	local lmCache = ents.LightMapComponent.DataCache()
	lmCache:SetLightmapEntity(self.m_lightmapEntityUuid)
	for ent,entMeshes in pairs(self.m_entities) do
		local hasRestructuredMeshes = false
		for _,meshInfo in ipairs(entMeshes) do
			if(meshInfo.restructuredMesh) then
				hasRestructuredMeshes = true
				break
			end
		end
		if(hasRestructuredMeshes == true) then
			local mdl = ent:GetModel()
			models[mdl:GetName()] = mdl
		end
		for _,meshInfo in ipairs(entMeshes) do
			local origMesh = meshInfo.origMesh
			local subMesh = meshInfo.subMesh
			local atlasMesh = (meshInfo.xatlasMeshIndex ~= nil) and meshes[meshInfo.xatlasMeshIndex] or nil
			local numBaseVerts = (atlasMesh ~= nil) and atlasMesh:GetVertexCount() or 0
			local lightmapUvs = {}

			local exMesh = meshInfo.exMesh
			local numExVerts = (exMesh ~= nil) and exMesh:GetVertexCount() or 0
			local numVerts = numBaseVerts +numExVerts
			local dsVerts = util.DataStream()
			dsVerts:Resize(numVerts *(util.SIZEOF_VECTOR3 *2 +util.SIZEOF_VECTOR2 +util.SIZEOF_VECTOR4))
			for j=1,numBaseVerts do
				local atlasData = atlasMesh:GetVertex(j -1)
				local uv = atlasData.uv
				local originalVertexIndex = atlasData.originalVertexIndex
				local oldVertex = subMesh:GetVertex(originalVertexIndex)
				local newVertex = oldVertex:Copy()
				table.insert(lightmapUvs,uv)

				dsVerts:WriteVector(oldVertex.position)
				dsVerts:WriteVector2(oldVertex.uv)
				dsVerts:WriteVector(oldVertex.normal)
				dsVerts:WriteVector4(oldVertex.tangent)
			end

			if(numExVerts > 0) then
				for j=1,exMesh:GetVertexCount() do
					local v = exMesh:GetVertex(j -1)
					table.insert(lightmapUvs,Vector2())

					dsVerts:WriteVector(v.position)
					dsVerts:WriteVector2(v.uv)
					dsVerts:WriteVector(v.normal)
					dsVerts:WriteVector4(v.tangent)
				end
			end

			lmCache:AddInstanceData(ent:GetUuid(),ent:GetModelName(),ent:GetPose(),origMesh:GetUuid(),lightmapUvs)

			if(meshInfo.restructuredMesh == true) then
				--subMesh:ClearVertices()
				--subMesh:ClearUVSets()
				--subMesh:SetVertexCount(#newVerts)
				--subMesh:AddUVSet("lightmap")
				--[[for j,v in ipairs(newVerts) do
					subMesh:SetVertex(j -1,v)
					subMesh:SetVertexUV("lightmap",j -1,Vector2(lightmapUvs[j].x,lightmapUvs[j].y))
				end]]

				--subMesh:ClearIndices()
				local extData = origMesh:GetExtensionData()
				local udmLightmapData = extData:Get("lightmapData")
				local udmMeshData = udmLightmapData:Get("meshData")
				local strct = udm.define_struct({
					{
						type = udm.TYPE_VECTOR3,
						name = "pos"
					},
					{
						type = udm.TYPE_VECTOR2,
						name = "uv"
					},
					{
						type = udm.TYPE_VECTOR3,
						name = "n"
					},
					{
						type = udm.TYPE_VECTOR4,
						name = "t"
					}
				})
				if(atlasMesh ~= nil) then
					local indices = {}
					local numIndices = atlasMesh:GetIndexCount()
					for i=1,numIndices,3 do
						local triIndices = {atlasMesh:GetIndex(i -1),atlasMesh:GetIndex(i),atlasMesh:GetIndex(i +1)}
						--subMesh:AddTriangle(triIndices[1],triIndices[2],triIndices[3])

						for _,idx in ipairs(triIndices) do table.insert(indices,idx) end
					end
					local exIndexOffset = numBaseVerts
					if(exMesh ~= nil) then for _,idx in ipairs(exMesh:GetIndices()) do table.insert(indices,exIndexOffset +idx) end end
					--subMesh:Update(game.Model.FUPDATE_ALL)

					local maxIndex = -1
					for _,idx in ipairs(indices) do
						maxIndex = math.max(maxIndex,idx)
					end
					local indexType = (maxIndex > util.MAX_UINT16) and game.Model.Mesh.Sub.INDEX_TYPE_UINT32 or game.Model.Mesh.Sub.INDEX_TYPE_UINT16

					udmMeshData:SetArrayValues("vertices",strct,numVerts,dsVerts,udm.TYPE_ARRAY_LZ4)
					udmMeshData:SetArrayValues("indices",(indexType == game.Model.Mesh.Sub.INDEX_TYPE_UINT32) and udm.TYPE_UINT32 or udm.TYPE_UINT16,indices,udm.TYPE_ARRAY_LZ4)
				else
					udmMeshData:SetArrayValues("vertices",strct,0,dsVerts,udm.TYPE_ARRAY_LZ4)
					udmMeshData:SetArrayValues("indices",udm.TYPE_UINT16,{},udm.TYPE_ARRAY_LZ4)
				end
			end
		end
	end

	-- We need to save the models because we changed the extension data
	for mdlName,mdl in pairs(models) do
		mdl:Save()
	end

	for ent,_ in pairs(self.m_entities) do
		if(ent:IsValid()) then
			local mdl = ent:GetModel()
			if(mdl ~= nil and models[mdl:GetName()] ~= nil) then mdl:Update() end
		end
	end
	file.remove_file_extension(lightmapCachePath,{"lmd","lmd_b"})
	lightmapCachePath = lightmapCachePath .. ".lmd_b"
	lmCache:SaveAs(lightmapCachePath)
end
