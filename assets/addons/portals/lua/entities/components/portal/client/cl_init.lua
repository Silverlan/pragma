--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/shaders/reflection/reflection.lua")
include("../shared.lua")

local Component = ents.PortalComponent
function Component:SetDebugCameraFrozen(frozen)
	if frozen == false then
		self.m_frozenCamData = nil
		return
	end
	local gameScene = game.get_scene()
	local gameCam = gameScene:GetActiveCamera()
	self.m_frozenCamData = {
		viewMatrix = gameCam:GetViewMatrix(),
		pose = gameCam:GetEntity():GetPose(),
	}
end

function Component:UpdateCamera(renderTargetC)
	local surfC = self:GetEntity():GetComponent(ents.COMPONENT_SURFACE)
	if surfC == nil then
		return
	end
	local plane = surfC:GetPlaneWs()

	local gameScene = game.get_scene()
	local gameCam = gameScene:GetActiveCamera()
	if gameCam == nil then
		return
	end
	local vm
	local camPose
	if self.m_frozenCamData ~= nil then
		vm = self.m_frozenCamData.viewMatrix:Copy()
		camPose = self.m_frozenCamData.pose:Copy()
	else
		vm = gameCam:GetViewMatrix()
		camPose = gameCam:GetEntity():GetPose()
	end

	local srcPos = self:GetSurfacePose():GetOrigin()
	local targetPos = self:GetTargetPose():GetOrigin()
	local srcRot = self:GetSurfacePose():GetRotation()
	local targetRot = self:GetTargetPose():GetRotation()

	local targetMatrix = Mat4(1.0)
	local mirror = self:IsMirrored()

	local offset = targetPos - srcPos
	if mirror then
		offset.x = -offset.x
	else
		offset = -offset
	end
	targetMatrix:Translate(offset)
	local targetRotMat = Mat4(1.0)

	targetRotMat:Rotate(srcRot:GetInverse() * targetRot)
	vm:Translate(srcPos)

	local tmp = Vector(vm:Get(3, 0), vm:Get(3, 1), vm:Get(3, 2))
	vm:Set(3, 0, 0)
	vm:Set(3, 1, 0)
	vm:Set(3, 1, 0) -- Temporarily disable translation, since we don't want to affect it
	vm = vm * targetRotMat
	vm:Set(3, 0, tmp.x)
	vm:Set(3, 1, tmp.y)
	vm:Set(3, 2, tmp.z)

	vm = vm * targetMatrix
	if mirror then
		local n = plane:GetNormal()
		vm = vm * matrix.create_reflection(n, 0.0)
	end
	vm:Translate(-srcPos)

	local cam = renderTargetC:GetCamera()
	cam:SetViewMatrix(vm)

	local side = geometry.get_side_of_point_to_plane(plane:GetNormal(), plane:GetDistance(), camPose:GetOrigin())
	local newCamPose = self:ProjectPoseToTarget(camPose, side)
	cam:GetEntity():SetPose(newCamPose)

	renderTargetC:GetDrawSceneInfo().pvsOrigin = camPose:GetOrigin()
end
