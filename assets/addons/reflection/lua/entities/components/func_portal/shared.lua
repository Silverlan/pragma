--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Component = util.register_class("ents.FuncPortalComponent",BaseEntityComponent)

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:AddEntityComponent("portal")
	self:BindEvent(ents.ModelComponent.EVENT_ON_MODEL_CHANGED,"InitializeReflectionPlane")
	if(CLIENT) then
		self:BindEvent(ents.PortalComponent.EVENT_ON_RENDER_SCENE_INITIALIZED,"InitializeReflectionMaterial")
	end
end

function Component:InitializeReflectionPlane()
	local meshInfo = self:FindReflectionMesh()
	if(meshInfo == nil) then return end
	self.m_meshInfo = meshInfo
	self:UpdatePlane()
end

function Component:OnPoseChanged()
	self:UpdatePlane()
end

function Component:UpdatePlane()
	if(self.m_meshInfo == nil) then return end
	local mdl = self:GetEntity():GetModel()
	local subMesh = mdl:GetMeshGroup(self.m_meshInfo[1]):GetMesh(self.m_meshInfo[2]):GetSubMesh(self.m_meshInfo[3])
	local idx0,idx1,idx2 = subMesh:GetTriangle(0)
	if(idx0 == nil) then return end
	local pose = self:GetEntity():GetPose()
	self.m_plane = math.Plane(
		pose *subMesh:GetVertexPosition(idx0),
		pose *subMesh:GetVertexPosition(idx1),
		pose *subMesh:GetVertexPosition(idx2)
	)
	local portalC = self:GetEntity():GetComponent(ents.COMPONENT_PORTAL)
	if(portalC ~= nil) then
		portalC:SetReflectionPlane(self.m_plane)
	end
	return self.m_plane
end

function Component:FindReflectionMesh()
	local mdl = self:GetEntity():GetModel()
	if(mdl == nil) then return end
	local candidate
	for mgIdx,meshGroup in ipairs(mdl:GetMeshGroups()) do
		for mIdx,mesh in ipairs(meshGroup:GetMeshes()) do
			for smIdx,subMesh in ipairs(mesh:GetSubMeshes()) do
				local idx = subMesh:GetSkinTextureIndex()
				local mat = mdl:GetMaterial(idx)
				if(util.is_valid(mat) and mat:GetShaderName() ~= "nodraw") then
					candidate = {mgIdx -1,mIdx -1,smIdx -1}
					if(mat:GetShaderName() == "reflection") then return candidate end
				end
			end
		end
	end
	return candidate
end
ents.COMPONENT_FUNC_PORTAL = ents.register_component("func_portal",Component,ents.EntityComponent.FREGISTER_BIT_NETWORKED)
