-- SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

locale.load("toon_shader.txt")

local Component = util.register_class("ents.ToonComponent", BaseEntityComponent)
function Component:Initialize()
	BaseEntityComponent.Initialize(self)
	local outlineC = self:AddEntityComponent("pfm_outline")
	outlineC:SetOutlineColor(Color.Black)
	outlineC:SetOutlineWidth(0.002)
end

function Component:InitializeMaterialOverrides()
	local mdl = self:GetEntity():GetModel()
	if mdl == nil then
		return
	end
	local mdlC = self:GetEntity():GetModelComponent()
	local matOverrideC = self:GetEntity():GetComponent(ents.COMPONENT_MATERIAL_OVERRIDE)
	if matOverrideC ~= nil then
		local n = mdl:GetMaterialCount()
		for i = 0, n - 1 do
			local mat = mdl:GetMaterial(i)
			if mat ~= nil then
				local shader = mat:GetShaderName()
				if shader ~= "eye_legacy" then
					mat = mat:Copy()
					mat:SetShader("toon")
					matOverrideC:SetMaterialOverride(i, mat)
				end
			end
		end
	end
	if mdlC ~= nil then
		mdlC:UpdateRenderMeshes()
	end
end

function Component:OnRemove()
	local mdlC = self:GetEntity():GetModelComponent()
	if mdlC ~= nil then
		local matOverrideC = self:GetEntity():GetComponent(ents.COMPONENT_MATERIAL_OVERRIDE)
		if matOverrideC ~= nil then
			mdlC:ClearMaterialOverrides()
		end
		mdlC:UpdateRenderMeshes()
	end

	self:GetEntity():RemoveComponent("pfm_outline")
end

function Component:OnEntitySpawn()
	self:InitializeMaterialOverrides()
end
ents.register_component("toon", Component, "rendering/model/shaders")
