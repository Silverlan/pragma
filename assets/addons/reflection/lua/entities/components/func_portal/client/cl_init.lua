--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("../shared.lua")

local Component = ents.FuncPortalComponent

function Component:InitializeReflectionMaterial()
	local portalC = self:GetEntity():GetComponent(ents.COMPONENT_PORTAL)
	local renderer = portalC:GetReflectionRenderer()
	if(util.is_valid(renderer) == false or self.m_meshInfo == nil) then return end

	local mdl = self:GetEntity():GetModel()
	local subMesh = mdl:GetMeshGroup(self.m_meshInfo[1]):GetMesh(self.m_meshInfo[2]):GetSubMesh(self.m_meshInfo[3])
	local mat = game.create_material("reflection")
	mat:SetTexture("albedo_map",renderer:GetHDRPresentationTexture())
	mat:UpdateTextures()
	local matIdx = mdl:AddMaterial(0,mat)
	subMesh:SetSkinTextureIndex(matIdx)
end
