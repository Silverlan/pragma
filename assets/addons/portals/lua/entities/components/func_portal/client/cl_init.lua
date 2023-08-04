--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("../shared.lua")

local Component = ents.FuncPortalComponent

function Component:InitializeReflectionMaterial()
	local renderTargetC = self:GetEntity():GetComponent(ents.COMPONENT_RENDER_TARGET)
	local surfC = self:GetEntity():GetComponent(ents.COMPONENT_SURFACE)
	local renderer = renderTargetC:GetRenderer()
	if util.is_valid(renderer) == false or surfC == nil then
		return
	end
	local mesh = surfC:GetMesh()
	if mesh == nil then
		return
	end
	local mat = game.create_material("reflection")
	mat:SetTexture("albedo_map", renderer:GetHDRPresentationTexture())
	mat:UpdateTextures()
	local matIdx = self:GetEntity():GetModel():AddMaterial(0, mat)
	mesh:SetSkinTextureIndex(matIdx)

	local mdlC = self:GetEntity():GetComponent(ents.COMPONENT_MODEL)
	if mdlC ~= nil then
		mdlC:ReloadRenderBufferList(true)
	end
end
