-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

-- Based on Khronos implementation: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/d32ca25dc273c0b0982e29efcea01b45d0c85105/src/user_camera.js

include("/util/viewer_camera.lua")

local ViewerCamera = util.register_class("ents.ViewerCamera", BaseEntityComponent)

include("fly_to_target.lua")

function ViewerCamera:Initialize()
	BaseEntityComponent.Initialize(self)

	self.m_lookAtTarget = Vector()
	self.m_zoom = 1.0
	self.m_up = Vector(0, 1, 0)
	self.m_xRot = 0.0
	self.m_yRot = 0.0
	self.m_scaleFactor = 1.0
	self.m_tLastUpdate = time.real_time()

	self:AddEntityComponent(ents.COMPONENT_TRANSFORM)
	self:AddEntityComponent(ents.COMPONENT_TOGGLE)
	self:AddEntityComponent(ents.COMPONENT_CAMERA)
end

function ViewerCamera:OnRemove()
	util.remove(self.m_cbFlyToTarget)
end

function ViewerCamera:GetLastUpdateTime()
	return self.m_tLastUpdate
end

function ViewerCamera:SetZoom(zoom)
	self.m_zoom = zoom

	self:UpdatePosition()
	self:UpdatePose()
end
function ViewerCamera:GetZoom()
	return self.m_zoom
end

function ViewerCamera:GetLookAtTarget()
	return self.m_lookAtTarget
end
function ViewerCamera:SetLookAtTarget(tgt)
	self.m_lookAtTarget = tgt
end

function ViewerCamera:GetRotation()
	return self.m_xRot, self.m_yRot
end
function ViewerCamera:SetRotation(xRot, yRot)
	self.m_xRot = xRot
	self.m_yRot = yRot
	self.m_tLastUpdate = time.real_time()
end

function ViewerCamera:Rotate(x, y)
	local yMax = math.pi / 2.0 - 0.01
	self.m_xRot = self.m_xRot + math.rad(x)
	self.m_yRot = self.m_yRot + math.rad(y)
	self.m_yRot = math.clamp(self.m_yRot, -yMax, yMax)

	self:UpdatePose()
end

function ViewerCamera:UpdatePose()
	self:UpdatePosition()
	local position = self:GetEntity():GetPos()
	local target = self:GetLookAtTarget()
	local diff = target - position
	local l = diff:Length()
	local dir = vector.FORWARD
	if l > 0.0001 then
		dir = diff / l
	end
	local rot = Quaternion(dir, self.m_up)
	self:GetEntity():SetRotation(rot)

	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC ~= nil then
		camC:UpdateViewMatrix()
	end

	self.m_tLastUpdate = time.real_time()
	self:BroadcastEvent(ViewerCamera.EVENT_ON_CAMERA_UPDATED, { camC })
end

function ViewerCamera:GetPivotPos()
	local ent = self:GetEntity()
	local pos = ent:GetPos()
	return pos + ent:GetForward() * self:GetZoom()
end

function ViewerCamera:SyncPose()
	self:SetPose(self:GetEntity():GetPose())
end

function ViewerCamera:SetPose(pose)
	local rot = pose:GetRotation()
	local dir = rot:GetForward()
	local dist = self:GetZoom()
	local posTgt = pose:GetOrigin() + dir * dist
	self:SetLookAtTarget(posTgt)

	local ang = rot:ToEulerAngles()
	self.m_xRot = math.rad(180 - ang.y)
	self.m_yRot = math.rad(ang.p)
end

function ViewerCamera:ToLocalRotation(vector)
	util.viewer_camera.rotate_around(vector, self.m_yRot, self.m_xRot)
end

function ViewerCamera:UpdatePosition()
	local pos = util.viewer_camera.calc_camera_position(self.m_lookAtTarget, self.m_zoom, self.m_yRot, self.m_xRot)
	self:GetEntity():SetPos(pos)
	self.m_tLastUpdate = time.real_time()
end

function ViewerCamera:Pan(x, y)
	local moveSpeed = 1 / (self.m_scaleFactor * 200)

	local left = Vector(-1, 0, 0)
	self:ToLocalRotation(left)
	left = left * (x * moveSpeed)

	local up = Vector(0, 1, 0)
	self:ToLocalRotation(up)
	up = up * (y * moveSpeed)

	self.m_lookAtTarget = self.m_lookAtTarget + up
	self.m_lookAtTarget = self.m_lookAtTarget + left

	self:UpdatePose()
end

function ViewerCamera:GetSceneExtents()
	return util.viewer_camera.get_view_extents(self.m_target)
end
function ViewerCamera:FitCameraTargetToExtents(min, max)
	self.m_lookAtTarget = (min + max) / 2.0

	self:UpdatePose()
end
function ViewerCamera:GetFittingZoom(axisLength)
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC == nil then
		return 0.0
	end
	return util.viewer_camera.calc_fitting_distance(axisLength, camC:GetFOV(), camC:GetAspectRatio())
end
function ViewerCamera:FitZoomToExtents(min, max)
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	self.m_zoom = (camC ~= nil) and util.viewer_camera.calc_fitting_distance_from_extents(min, max, camC:GetFOV(), camC:GetAspectRatio()) or 0.0

	self:UpdatePose()
end
function ViewerCamera:FitViewToScene(min, max)
	if min == nil then
		min, max = self:GetSceneExtents()
	end
	self:FitCameraTargetToExtents(min, max)
	self:FitZoomToExtents(min, max)

	-- Zoom out a little bit
	self:SetZoom(self:GetZoom() + 15.0)
	return min, max
end
function ViewerCamera:SetTarget(target)
	self.m_target = target
	self:FitViewToScene()
	self:SetRotation(0.0, 0.0)

	self:Rotate(-35, 15)
end
function ViewerCamera:GetTarget()
	return self.m_target
end
function ViewerCamera:SetFovY(fov)
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC == nil then
		return
	end
	camC:SetFOV(math.vertical_fov_to_horizontal_fov(fov, self.m_width, self.m_height))
	camC:UpdateProjectionMatrix()
	self.m_tLastUpdate = time.real_time()
end
function ViewerCamera:Setup(width, height)
	self.m_width = width
	self.m_height = height
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC == nil then
		return
	end
	camC:SetAspectRatio(width / height)
	camC:UpdateViewMatrix()
	camC:UpdateProjectionMatrix()
	self.m_tLastUpdate = time.real_time()

	self:BroadcastEvent(ViewerCamera.EVENT_ON_CAMERA_UPDATED, { camC })
end
function ViewerCamera:OnEntitySpawn() end
ents.register_component("viewer_camera", ViewerCamera, "util")
ViewerCamera.EVENT_ON_CAMERA_UPDATED =
	ents.register_component_event(ents.COMPONENT_VIEWER_CAMERA, "on_camera_updated")
