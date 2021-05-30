--[[
    Copyright (C) 2019  Florian Weischer

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("util.UVAtlasGenerator")
function util.UVAtlasGenerator:__init()
	local r = engine.load_library("pr_xatlas")
	if(r ~= true) then
		console.print_warning("Unable to load openvr module: " .. r)
		return
	end
	self.m_atlas = xatlas.create()
	self.m_entities = {}
	self.m_numInputMeshes = 0
end

function util.UVAtlasGenerator:AddEntity(ent,meshFilter)
	local mdl = ent:GetModel()
	local mdlC = ent:GetComponent(ents.COMPONENT_MODEL)
	local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
	if(mdl == nil or mdlC == nil or renderC == nil) then return end
	local name = mdl:GetName()
	if(self.m_entities[ent] ~= nil) then return end
	mdl = mdl:Copy(game.Model.FCOPY_DEEP)

	local skin = mdlC:GetSkin()
	local bodyGroups = mdlC:GetBodyGroups()
	ent:SetModel(mdl)
	mdlC:SetSkin(skin)
	mdlC:SetBodyGroups(bodyGroups)

	self.m_entities[ent] = {}
	local renderMeshes = renderC:GetLODMeshes()
	for _,mesh in ipairs(renderMeshes) do
		for _,subMesh in ipairs(mesh:GetSubMeshes()) do
			-- if(subMesh:HasUVSet("lightmap")) then
				local mat = mdl:GetMaterial(subMesh:GetSkinTextureIndex()) -- mdlC:GetRenderMaterial(subMesh:GetSkinTextureIndex())
				if(mat ~= nil and (meshFilter == nil or meshFilter(mesh,subMesh))) then
					self.m_atlas:AddMesh(subMesh,mat)
					self.m_numInputMeshes = self.m_numInputMeshes +1
					table.insert(self.m_entities[ent],{
						subMesh = subMesh,
						xatlasMeshIndex = self.m_numInputMeshes
					})
				end
			-- end
		end
	end
end

function util.UVAtlasGenerator:Generate()
	local meshes = self.m_atlas:Generate()
	if(#meshes ~= self.m_numInputMeshes) then
		error("Number of output meshes doesn't match number of input meshes!")
		return
	end
	-- Clear buffers (to free up memory)
	for ent,meshes in pairs(self.m_entities) do
		if(ent:IsValid()) then
			for _,meshInfo in ipairs(meshes) do
				local mesh = meshInfo.subMesh
				local sceneMesh = mesh:GetSceneMesh()
				if(sceneMesh ~= nil) then sceneMesh:ClearBuffers() end
			end
		end
	end

	for ent,entMeshes in pairs(self.m_entities) do
		for _,meshInfo in ipairs(entMeshes) do
			local origMesh = meshInfo.subMesh
			local atlasMesh = meshes[meshInfo.xatlasMeshIndex]
			local numVerts = atlasMesh:GetVertexCount()
			local newVerts = {}
			local lightmapUvs = {}
			for j=1,numVerts do
				local atlasData = atlasMesh:GetVertex(j -1)
				local uv = atlasData.uv
				local originalVertexIndex = atlasData.originalVertexIndex
				local oldVertex = origMesh:GetVertex(originalVertexIndex)
				local newVertex = oldVertex:Copy()
				table.insert(newVerts,newVertex)
				table.insert(lightmapUvs,uv)
			end
			origMesh:ClearVertices()
			origMesh:ClearUVSets()
			origMesh:SetVertexCount(#newVerts)
			origMesh:AddUVSet("lightmap")
			for j,v in ipairs(newVerts) do
				origMesh:SetVertex(j -1,v)
				origMesh:SetVertexUV("lightmap",j -1,Vector2(lightmapUvs[j].x,lightmapUvs[j].y))
			end

			origMesh:ClearTriangles()
			local numIndices = atlasMesh:GetIndexCount()
			for i=1,numIndices,3 do
				origMesh:AddTriangle(atlasMesh:GetIndex(i -1),atlasMesh:GetIndex(i),atlasMesh:GetIndex(i +1))
			end
			origMesh:Update(game.Model.FUPDATE_ALL)
		end
	end

	for ent,_ in pairs(self.m_entities) do
		if(ent:IsValid()) then
			local mdl = ent:GetModel()
			if(mdl ~= nil) then mdl:Update() end
		end
	end
end
