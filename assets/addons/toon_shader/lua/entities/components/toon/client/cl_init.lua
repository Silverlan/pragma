--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

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
	local n = mdl:GetMaterialCount()
	for i = 0, n - 1 do
		local mat = mdl:GetMaterial(i)
		if mat ~= nil then
			local shader = mat:GetShaderName()
			if shader ~= "eye_legacy" then
				mat = mat:Copy()
				mat:SetShader("toon")
				mdlC:SetMaterialOverride(i, mat)
			end
		end
	end
	mdlC:UpdateRenderMeshes()
end

function Component:OnRemove()
	local mdlC = self:GetEntity():GetModelComponent()
	if mdlC ~= nil then
		mdlC:ClearMaterialOverrides()
		mdlC:UpdateRenderMeshes()
	end

	self:GetEntity():RemoveComponent("pfm_outline")
end

function Component:OnEntitySpawn()
	self:InitializeMaterialOverrides()
end
ents.register_component("toon", Component, "rendering/model/shaders")
