-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local Component = util.register_class("ents.FuncPortalComponent", BaseEntityComponent)

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:AddEntityComponent("portal")
	self:BindEvent(ents.ModelComponent.EVENT_ON_MODEL_CHANGED, "InitializeReflectionPlane")
	if CLIENT then
		self:BindEvent(ents.RenderTargetComponent.EVENT_ON_RENDER_SCENE_INITIALIZED, "InitializeReflectionMaterial")
	end
end

function Component:InitializeReflectionPlane()
	local surfC = self:GetEntity():GetComponent(ents.COMPONENT_SURFACE)
	if surfC == nil then
		return
	end
	surfC:FindAndAssignSurfaceMesh(function(mat, shader)
		if shader == "nodraw" then
			return -1
		end
		return shader == "reflection" and 1 or 0
	end)
end
ents.COMPONENT_FUNC_PORTAL =
	ents.register_component("func_portal", Component, ents.EntityComponent.FREGISTER_BIT_NETWORKED)
