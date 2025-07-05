-- SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include("/shaders/example_simple_game_shader/shader.lua")
include("/shaders/example_wireframe/shader.lua")

local Component = util.register_class("ents.ExampleShaderComponent", BaseEntityComponent)

function Component:Initialize()
	BaseEntityComponent.Initialize(self)
	self:AddEntityComponent(ents.COMPONENT_RENDER)
	self:AddEntityComponent(ents.COMPONENT_MODEL)
	self:InitializeMaterial()
end

function Component:InitializeMaterial()
	local mat = game.create_material("pbr")
	mat:SetTexture("albedo_map", "white")
	mat:UpdateTextures()
	self.m_material = mat
end

function Component:InitializeMaterialOverrides()
	local mdl = self:GetEntity():GetModel()
	if mdl == nil then
		return
	end
	local mdlC = self:GetEntity():GetModelComponent()
	local n = mdl:GetMaterialCount()
	for i = 0, n - 1 do
		mdlC:SetMaterialOverride(i, self.m_material)
	end
	mdlC:UpdateRenderMeshes()
end

function Component:OnRemove() end

function Component:SetShader(shader)
	self.m_material:SetShader(shader)
	local mdlC = self:GetEntity():GetModelComponent()
	if mdlC ~= nil then
		mdlC:ReloadRenderBufferList(true)
	end
end

function Component:OnEntitySpawn()
	self:InitializeMaterialOverrides()
end
ents.COMPONENT_EXAMPLE_SHADER = ents.register_component("example_shader", Component)
