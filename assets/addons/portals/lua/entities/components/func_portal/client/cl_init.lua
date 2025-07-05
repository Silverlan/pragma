-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

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
