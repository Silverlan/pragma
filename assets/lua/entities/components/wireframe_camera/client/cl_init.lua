--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/shaders/pfm/pfm_wireframe_line.lua")

util.register_class("ents.WireframeCameraComponent", BaseEntityComponent)
local Component = ents.WireframeCameraComponent
function Component:Initialize()
	BaseEntityComponent.Initialize(self)
end

function Component:OnEntitySpawn()
	self:SetFrustumModelVisible(true)
end

function Component:OnRemove()
	if self.m_modelApplied == true then
		local mdlC = self:GetEntity():GetComponent(ents.COMPONENT_MODEL)
		if mdlC ~= nil then
			mdlC:ClearModel()
		end
	end
end

local MODEL_VERTEX_COUNT = 32
function Component:InitializeModel()
	if self.m_frustumModel ~= nil then
		return self.m_frustumModel
	end
	-- Generate model
	local mdl = game.create_model()
	local meshGroup = mdl:GetMeshGroup(0)

	local subMesh = game.Model.Mesh.Sub.create()
	subMesh:SetGeometryType(game.Model.Mesh.Sub.GEOMETRY_TYPE_LINES)

	local indices = {}
	for i = 1, MODEL_VERTEX_COUNT do
		local v = game.Model.Vertex(Vector(), Vector2(0, 0), Vector(0, 0, 0))
		local idx = subMesh:AddVertex(v)
		table.insert(indices, idx)
	end

	for i = 1, #indices, 2 do
		subMesh:AddLine(indices[i], indices[i + 1])
	end

	local mat = game.create_material("pfm_wireframe_line")
	mat:SetTexture("albedo_map", "white")
	local matIdx = mdl:AddMaterial(0, mat)
	subMesh:SetSkinTextureIndex(matIdx)

	local mesh = game.Model.Mesh.Create()
	mesh:AddSubMesh(subMesh)
	meshGroup:AddMesh(mesh)

	mdl:Update(game.Model.FUPDATE_ALL)
	mdl:SetRenderBounds(Vector(-10000, -10000, -10000), Vector(10000, 10000, 10000)) -- TODO: What should we set these to?
	self.m_frustumModel = mdl
	self:UpdateModel()
	self:SetFrustumModelDirty()

	local renderC = self:GetEntity():GetComponent(ents.COMPONENT_RENDER)
	if renderC ~= nil then
		renderC:SetExemptFromOcclusionCulling(true)
	end
	return mdl
end
function Component:UpdateModel()
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC == nil or self.m_frustumModel == nil then
		return
	end
	local meshGroup = self.m_frustumModel:GetMeshGroup(0) or nil
	local mesh = (meshGroup ~= nil) and meshGroup:GetMesh(0) or nil
	local subMesh = (mesh ~= nil) and mesh:GetSubMesh(0) or nil
	if subMesh == nil then
		return
	end

	local pos = Vector()
	local fov = camC:GetFOVRad()
	local nearPlaneBoundaries =
		math.get_frustum_plane_boundaries(pos, vector.FORWARD, vector.UP, fov, camC:GetAspectRatio(), camC:GetNearZ())
	local farPlaneBoundaries =
		math.get_frustum_plane_boundaries(pos, vector.FORWARD, vector.UP, fov, camC:GetAspectRatio(), camC:GetFarZ())
	local vertIdx = 0

	-- Near plane
	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[1])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[2])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[2])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[3])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[3])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[4])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[4])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, nearPlaneBoundaries[1])
	vertIdx = vertIdx + 1

	-- Far plane
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[1])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[2])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[2])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[3])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[3])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[4])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[4])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[1])
	vertIdx = vertIdx + 1

	-- Cam pos to far plane
	subMesh:SetVertexPosition(vertIdx, pos)
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[1])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, pos)
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[2])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, pos)
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[3])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, pos)
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, farPlaneBoundaries[4])
	vertIdx = vertIdx + 1

	-- Focal distance plane
	local focalDistance = 20 -- self:GetCameraData():GetFocalDistance() -- TODO
	local focalPlaneBoundaries =
		math.get_frustum_plane_boundaries(pos, vector.FORWARD, vector.UP, fov, camC:GetAspectRatio(), focalDistance)

	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[1])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[2])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[2])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[3])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[3])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[4])
	vertIdx = vertIdx + 1

	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[4])
	vertIdx = vertIdx + 1
	subMesh:SetVertexPosition(vertIdx, focalPlaneBoundaries[1])
	vertIdx = vertIdx + 1

	subMesh:Update(game.Model.FUPDATE_VERTEX_BUFFER)
end
function Component:SetFrustumModelVisible(visible)
	if visible then
		self:GetEntity():AddComponent(ents.COMPONENT_RENDER)
	end

	if visible == false then
		return
	end
	local mdlC = self:GetEntity():AddComponent(ents.COMPONENT_MODEL)
	if mdlC == nil or mdlC:GetModel() ~= nil then
		return
	end
	local model = self:InitializeModel()
	if model == nil then
		return
	end
	mdlC:SetModel(model)
	self.m_modelApplied = true
end
function Component:OnUpdateRenderData()
	if self.m_updateFrustumModel ~= true then
		return
	end
	self.m_updateFrustumModel = nil
	self:UpdateModel()
end
function Component:SetFrustumModelDirty()
	self.m_updateFrustumModel = true
end
ents.register_component("wireframe_camera", Component, "editor", ents.EntityComponent.FREGISTER_BIT_HIDE_IN_EDITOR)
