--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("ents.DebugModelMesh", BaseEntityComponent)
local Component = ents.DebugModelMesh

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
end

function Component:OnEntitySpawn()
	self:UpdateDebugMesh()
end

function Component:OnRemove()
	util.remove(self.m_dbgObject)
end

function Component:OnTick()
	if util.is_valid(self.m_dbgObject) then
		self.m_dbgObject:SetPose(self:GetEntity():GetPose())
	end
end

function Component:UpdateDebugMesh()
	util.remove(self.m_dbgObject)
	local mdl = self:GetEntity():GetModel()
	if mdl == nil then
		return
	end
	local dbgTris = {}
	for _, meshGroup in ipairs(mdl:GetMeshGroups()) do
		for _, mesh in ipairs(meshGroup:GetMeshes()) do
			for _, subMesh in ipairs(mesh:GetSubMeshes()) do
				local verts = subMesh:GetVertices()
				local tris = subMesh:GetIndices()
				for _, idx in ipairs(tris) do
					table.insert(dbgTris, verts[idx + 1])
				end
			end
		end
	end
	local drawInfo = debug.DrawInfo()
	drawInfo:SetColor(Color(255, 0, 0, 64))
	drawInfo:SetOutlineColor(Color.White)
	self.m_dbgObject = debug.draw_mesh(dbgTris, drawInfo)
	if util.is_valid(self.m_dbgObject) == false then
		return
	end
	self.m_dbgObject:SetPose(self:GetEntity():GetPose())
end
ents.COMPONENT_DEBUG_MODEL_MESH = ents.register_component("debug_model_mesh", Component)
