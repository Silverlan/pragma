util.register_class("util.UVAtlasGenerator")
function util.UVAtlasGenerator:__init(lmUuid)
	local r = engine.load_library("pr_xatlas")
	if r ~= true then
		console.print_warning("Unable to load openvr module: " .. r)
		return
	end
	self.m_atlas = xatlas.create()
	self.m_entities = {}
	self.m_lightmapEntityUuid = lmUuid
	self.m_numInputMeshes = 0
end

function util.UVAtlasGenerator:AddEntity(ent, meshFilter)
	local mdl = ent:GetModel()
	local mdlC = ent:GetComponent(ents.COMPONENT_MODEL)
	local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
	if mdl == nil or mdlC == nil or renderC == nil then
		return
	end
	mdl = game.load_model(mdl:GetName()) -- Get original model
	if mdl == nil then
		return
	end
	if self.m_entities[ent] ~= nil then
		return
	end
	mdl = mdl:Copy(bit.bor(game.Model.FCOPY_DEEP, game.Model.FCOPY_BIT_COPY_UNIQUE_IDS))

	local skin = mdlC:GetSkin()
	local bodyGroups = mdlC:GetBodyGroups()
	ent:SetModel(mdl)
	mdlC:SetSkin(skin)
	mdlC:SetBodyGroups(bodyGroups)

	self.m_entities[ent] = {}
	for _, subMesh in ipairs(renderC:GetRenderMeshes()) do
		-- if(subMesh:HasUVSet("lightmap")) then
		local mat = mdl:GetMaterial(subMesh:GetSkinTextureIndex()) -- mdlC:GetRenderMaterial(subMesh:GetSkinTextureIndex())
		if mat ~= nil and (meshFilter == nil or meshFilter(mesh, subMesh)) then
			self.m_atlas:AddMesh(subMesh, mat, ent:GetScale())
			self.m_numInputMeshes = self.m_numInputMeshes + 1
			table.insert(self.m_entities[ent], {
				subMesh = subMesh,
				xatlasMeshIndex = self.m_numInputMeshes,
			})
		end
		-- end
	end
end

function util.UVAtlasGenerator:Generate(lightmapCachePath)
	local meshes = self.m_atlas:Generate()
	if #meshes ~= self.m_numInputMeshes then
		error("Number of output meshes doesn't match number of input meshes!")
		return
	end

	for ent, entMeshes in pairs(self.m_entities) do
		if ent:IsValid() then
			for _, meshInfo in ipairs(entMeshes) do
				local mesh = meshInfo.subMesh
				local atlasMesh = meshes[meshInfo.xatlasMeshIndex]
				local origIndexCount = mesh:GetIndexCount()
				local newIndexCount = atlasMesh:GetIndexCount()
				meshInfo.restructuredMesh = true -- (newIndexCount ~= origIndexCount)
			end
		end
	end

	-- Clear buffers (to free up memory)
	for ent, meshes in pairs(self.m_entities) do
		if ent:IsValid() then
			for _, meshInfo in ipairs(meshes) do
				if meshInfo.restructuredMesh == true then
					local mesh = meshInfo.subMesh
					local sceneMesh = mesh:GetSceneMesh()
					if sceneMesh ~= nil then
						sceneMesh:ClearBuffers()
					end
				end
			end
		end
	end

	local models = {}
	local lmCache = ents.LightMapComponent.DataCache()
	lmCache:SetLightmapEntity(self.m_lightmapEntityUuid)
	for ent, entMeshes in pairs(self.m_entities) do
		local hasRestructuredMeshes = false
		for _, meshInfo in ipairs(entMeshes) do
			if meshInfo.restructuredMesh then
				hasRestructuredMeshes = true
				break
			end
		end
		if hasRestructuredMeshes == true then
			local mdl = ent:GetModel()
			models[mdl:GetName()] = mdl
		end
		for _, meshInfo in ipairs(entMeshes) do
			local origMesh = meshInfo.subMesh
			local atlasMesh = meshes[meshInfo.xatlasMeshIndex]
			local numVerts = atlasMesh:GetVertexCount()
			local newVerts = {}
			local lightmapUvs = {}
			local dsVerts = util.DataStream()
			dsVerts:Resize(numVerts * (util.SIZEOF_VECTOR3 * 2 + util.SIZEOF_VECTOR2 + util.SIZEOF_VECTOR4))
			local numAlphas = origMesh:GetAlphaCount()
			local alphas
			if numAlphas > 0 then
				alphas = {}
			end
			for j = 1, numVerts do
				local atlasData = atlasMesh:GetVertex(j - 1)
				local uv = atlasData.uv
				local originalVertexIndex = atlasData.originalVertexIndex
				local oldVertex = origMesh:GetVertex(originalVertexIndex)
				local newVertex = oldVertex:Copy()
				table.insert(newVerts, newVertex)
				table.insert(lightmapUvs, uv)

				dsVerts:WriteVector(oldVertex.position)
				dsVerts:WriteVector2(oldVertex.uv)
				dsVerts:WriteVector(oldVertex.normal)
				dsVerts:WriteVector4(oldVertex.tangent)

				if alphas ~= nil then
					local oldAlpha = origMesh:GetVertexAlpha(originalVertexIndex)
					oldAlpha = oldAlpha or Vector2()
					table.insert(alphas, (numAlphas == 1) and oldAlpha.x or oldAlpha)
				end
			end

			lmCache:AddInstanceData(
				ent:GetUuid(),
				ent:GetModel():GetName(),
				ent:GetPose(),
				origMesh:GetUuid(),
				lightmapUvs
			)

			if meshInfo.restructuredMesh == true then
				--origMesh:ClearVertices()
				--origMesh:ClearUVSets()
				--origMesh:SetVertexCount(#newVerts)
				--origMesh:AddUVSet("lightmap")
				--[[for j,v in ipairs(newVerts) do
					origMesh:SetVertex(j -1,v)
					origMesh:SetVertexUV("lightmap",j -1,Vector2(lightmapUvs[j].x,lightmapUvs[j].y))
				end]]

				--origMesh:ClearIndices()
				local numIndices = atlasMesh:GetIndexCount()
				local indices = {}
				local maxIndex = -1
				for i = 1, numIndices do
					maxIndex = math.max(maxIndex, atlasMesh:GetIndex(i - 1))
				end
				local indexType = (maxIndex > util.MAX_UINT16) and game.Model.Mesh.Sub.INDEX_TYPE_UINT32
					or game.Model.Mesh.Sub.INDEX_TYPE_UINT16
				for i = 1, numIndices, 3 do
					local triIndices = { atlasMesh:GetIndex(i - 1), atlasMesh:GetIndex(i), atlasMesh:GetIndex(i + 1) }
					--origMesh:AddTriangle(triIndices[1],triIndices[2],triIndices[3])

					for _, idx in ipairs(triIndices) do
						table.insert(indices, idx)
					end
				end
				--origMesh:Update(game.Model.FUPDATE_ALL)

				local extData = origMesh:GetExtensionData()
				local udmLightmapData = extData:Get("lightmapData")
				local udmMeshData = udmLightmapData:Get("meshData")
				local strct = udm.define_struct({
					{
						type = udm.TYPE_VECTOR3,
						name = "pos",
					},
					{
						type = udm.TYPE_VECTOR2,
						name = "uv",
					},
					{
						type = udm.TYPE_VECTOR3,
						name = "n",
					},
					{
						type = udm.TYPE_VECTOR4,
						name = "t",
					},
				})
				udmMeshData:SetArrayValues("vertices", strct, numVerts, dsVerts, udm.TYPE_ARRAY_LZ4)
				if alphas ~= nil and #alphas > 0 then
					udmMeshData:RemoveValue("alphas")
					udmMeshData:SetArrayValues(
						"alphas",
						(numAlphas == 1) and udm.TYPE_FLOAT or udm.TYPE_VECTOR2,
						alphas,
						udm.TYPE_ARRAY_LZ4
					)
				end
				udmMeshData:SetArrayValues(
					"indices",
					(indexType == game.Model.Mesh.Sub.INDEX_TYPE_UINT32) and udm.TYPE_UINT32 or udm.TYPE_UINT16,
					indices,
					udm.TYPE_ARRAY_LZ4
				)
			end
		end
	end

	-- We need to save the models because we changed the extension data
	for mdlName, mdl in pairs(models) do
		mdl:Save()
		asset.reload(mdlName, asset.TYPE_MODEL)
	end

	for ent, _ in pairs(self.m_entities) do
		if ent:IsValid() then
			local mdl = ent:GetModel()
			local newMdl = asset.load(mdl:GetName(), asset.TYPE_MODEL)
			if newMdl ~= nil and models[newMdl:GetName()] ~= nil then
				newMdl:Update()

				local bodygroups = {}
				local skin = ent:GetSkin()
				local mdlC = ent:GetComponent(ents.COMPONENT_MODEL)
				if mdlC ~= nil then
					bodygroups = mdlC:GetBodyGroups()
				end
				ent:SetModel(newMdl)
				if util.is_valid(mdlC) then
					mdlC:SetSkin(skin)
					mdlC:SetBodyGroups(bodygroups)
				end
			end
		end
	end
	file.remove_file_extension(lightmapCachePath, { "lmd", "lmd_b" })
	lightmapCachePath = lightmapCachePath .. ".lmd_b"
	lmCache:SaveAs(lightmapCachePath)
end
