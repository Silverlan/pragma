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
	self.m_modelCache = {}
	self.m_inputMeshes = {}
end

function util.UVAtlasGenerator:AddEntity(ent,meshFilter)
	local mdl = ent:GetModel()
	local mdlC = ent:GetComponent(ents.COMPONENT_MODEL)
	local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
	if(mdl == nil or mdlC == nil or renderC == nil) then return end
	local name = mdl:GetName()
	for _,mdlOther in ipairs(self.m_modelCache) do
		if(mdlOther == mdl) then return end
	end
	table.insert(self.m_modelCache,mdl)
	local renderMeshes = renderC:GetLODMeshes()
	for _,mesh in ipairs(renderMeshes) do
		for _,subMesh in ipairs(mesh:GetSubMeshes()) do
			-- if(subMesh:HasUVSet("lightmap")) then
				local mat = mdl:GetMaterial(subMesh:GetSkinTextureIndex()) -- mdlC:GetRenderMaterial(subMesh:GetSkinTextureIndex())
				if(mat ~= nil and (meshFilter == nil or meshFilter(mesh,subMesh))) then
					self.m_atlas:AddMesh(subMesh,mat)
					table.insert(self.m_inputMeshes,subMesh)
				end
			-- end
		end
	end
end

function util.UVAtlasGenerator:Generate()
	local meshes = self.m_atlas:Generate()
	if(#meshes ~= #self.m_inputMeshes) then
		error("Number of output meshes doesn't match number of input meshes!")
		return
	end
	for i,origMesh in ipairs(self.m_inputMeshes) do
		local atlasMesh = meshes[i]
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
			--[[local uv = origMesh:GetVertexUV(j -1)
			if(uv.x > 1) then uv.x = uv.x -1.0 end
			if(uv.y > 1) then uv.y = uv.y -1.0 end
			origMesh:SetVertexUV(j -1,uv)]]
			-- origMesh:SetVertexUV("lightmap",j -1,origMesh:GetVertexUV(j -1))--Vector2(lightmapUvs[i].x,lightmapUvs[i].y))
		--	if(i == 2) then origMesh:SetVertexUV("lightmap",j -1,Vector2()) end
			--print("uv: ",Vector2(lightmapUvs[i].x,lightmapUvs[i].y))
		end

		origMesh:ClearTriangles()
		local numIndices = atlasMesh:GetIndexCount()
		for i=1,numIndices,3 do
			origMesh:AddTriangle(atlasMesh:GetIndex(i -1),atlasMesh:GetIndex(i),atlasMesh:GetIndex(i +1))
		end
		origMesh:Update(game.Model.FUPDATE_ALL)
	end

	for _,mdl in ipairs(self.m_modelCache) do
		mdl:Update()
	end
end
