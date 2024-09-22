--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Component = util.register_class("ents.PortalComponent", BaseEntityComponent)

Component:RegisterMember(
	"Target",
	ents.MEMBER_TYPE_ENTITY,
	"",
	{ onChange = CLIENT and function(self)
		self:UpdateTarget()
	end or nil },
	"def+trans"
)
Component:RegisterMember("PortalOrigin", ents.MEMBER_TYPE_VECTOR3, Vector(0, 0, 0), {
	onChange = function(self)
		self:UpdatePortalOrigin()
		self:InvokeEventCallbacks(ents.PortalComponent.EVENT_ON_PORTAL_ORIGIN_CHANGED)
	end,
}, "def+trans")
Component:RegisterMember("Mirrored", ents.MEMBER_TYPE_BOOLEAN, false, {
	onChange = function(self)
		self:UpdateMirrored()
	end,
}, "def+trans+is")
Component:RegisterMember("PortalOriginEnabled", ents.MEMBER_TYPE_BOOLEAN, true, {
	onChange = function(self)
		self.m_posesDirty = true
	end,
}, "def+trans+is")

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:AddEntityComponent(ents.COMPONENT_MODEL)
	self:AddEntityComponent(ents.COMPONENT_RENDER)
	self:AddEntityComponent(ents.COMPONENT_SURFACE)
	local renderTargetC = self:AddEntityComponent("render_target")
	self.m_relativePortalOrigin = Vector()
	self:UpdatePortalOrigin()
	if CLIENT then
		self:SetMirrored(false)

		renderTargetC:AddEventCallback(ents.RenderTargetComponent.EVENT_PRE_RENDER_SCENE, function(c)
			self:UpdateCamera(renderTargetC, c)
		end)
	end

	if SERVER then
		self:AddEntityComponent(ents.COMPONENT_PHYSICS)
		self:AddEntityComponent(ents.COMPONENT_TOUCH)
		self:SetTickPolicy(ents.TICK_POLICY_NEVER)

		self:BindEvent(ents.TouchComponent.EVENT_ON_START_TOUCH, "OnStartTouch")
		self:BindEvent(ents.TouchComponent.EVENT_ON_END_TOUCH, "OnEndTouch")
		self:BindEvent(ents.TouchComponent.EVENT_CAN_TRIGGER, "CanTrigger")
	end
	self:BindEvent(ents.TransformComponent.EVENT_ON_POSE_CHANGED, "OnPoseChanged")
	self:BindEvent(ents.SurfaceComponent.EVENT_ON_SURFACE_PLANE_CHANGED, function()
		self.m_posesDirty = true
	end)
end

function Component:OnEntitySpawn()
	self:UpdateTarget()
end

function Component:UpdateMirrored()
	if CLIENT then
		local renderTargetC = self:GetEntity():GetComponent(ents.COMPONENT_RENDER_TARGET)
		if renderTargetC ~= nil then
			local drawSceneInfo = renderTargetC:GetDrawSceneInfo()
			drawSceneInfo.flags = self:IsMirrored()
					and bit.bor(drawSceneInfo.flags, game.DrawSceneInfo.FLAG_REFLECTION_BIT)
				or bit.band(drawSceneInfo.flags, bit.bnot(game.DrawSceneInfo.FLAG_REFLECTION_BIT))
		end
		return
	end
	self:UpdateMirrorState()
end

function Component:OnPoseChanged()
	self:UpdatePortalOrigin()
end

function Component:UpdateTarget()
	self.m_posesDirty = true

	util.remove({ self.m_cbOnTargetPoseChanged, self.m_cbOnTargetPoseChanged })
	local target = self:GetTarget()
	local trC = util.is_valid(target) and target:GetComponent(ents.COMPONENT_TRANSFORM) or nil
	if trC == nil then
		return
	end
	self.m_cbOnTargetPoseChanged = trC:AddEventCallback(ents.TransformComponent.EVENT_ON_POSE_CHANGED, function()
		self.m_posesDirty = true
	end)

	local portalC = target:GetComponent(ents.COMPONENT_PORTAL)
	if portalC ~= nil then
		self.m_cbOnTargetPoseChanged = portalC:AddEventCallback(
			ents.PortalComponent.EVENT_ON_PORTAL_ORIGIN_CHANGED,
			function()
				self.m_posesDirty = true
			end
		)
	end
end

function Component:OnRemove()
	util.remove({ self.m_cbOnTargetPoseChanged, self.m_cbOnTargetPoseChanged })
end

function Component:UpdatePoses()
	if CLIENT then
		if self.m_t ~= engine.get_current_frame_index() then
			self.m_t = engine.get_current_frame_index()
			self.m_posesDirty = true
		end
	else
		if self.m_t ~= engine.get_tick_count() then
			self.m_t = engine.get_tick_count()
			self.m_posesDirty = true
		end
	end
	if self.m_posesDirty ~= true then
		return
	end
	self.m_posesDirty = nil

	local surfC = self:GetEntity():GetComponent(ents.COMPONENT_SURFACE)
	if surfC == nil then
		return
	end
	local srcPos = self:GetEntity():GetPose() * self:GetRelativePortalOrigin()
	local srcRot = surfC:GetPlaneRotation()
	local srcPose = math.Transform(srcPos, srcRot)

	local tgtPose = srcPose:Copy()
	local tgtPlane = surfC:GetPlane()

	local target = self:GetTarget()
	if util.is_valid(target) then
		local portalC = target:GetComponent(ents.COMPONENT_PORTAL)
		local surfCTgt = target:GetComponent(ents.COMPONENT_SURFACE)
		if portalC ~= nil and surfCTgt ~= nil then
			local tgtPos = target:GetPose() * portalC:GetRelativePortalOrigin()
			local tgtRot = surfCTgt:GetPlaneRotation()
			tgtPose = math.Transform(tgtPos, tgtRot)
			tgtPlane = surfCTgt:GetPlane()
		else
			tgtPose = target:GetPose()
			tgtPlane = tgtPose:ToPlane()
		end
	end
	self.m_surfacePose = srcPose
	self.m_targetPose = tgtPose
	self.m_targetPlane = tgtPlane

	if CLIENT then
		local renderTargetC = self:GetEntity():GetComponent(ents.COMPONENT_RENDER_TARGET)
		if renderTargetC ~= nil then
			local drawSceneInfo = renderTargetC:GetDrawSceneInfo()
			drawSceneInfo.clipPlane = -Vector4(tgtPlane:GetNormal(), tgtPlane:GetDistance())
		end
	end
end

function Component:UpdatePortalOrigin()
	local origin = self:GetPortalOrigin()
	self.m_relativePortalOrigin = self:GetEntity():GetPose():GetInverse() * origin
	self.m_posesDirty = true
end

function Component:GetRelativePortalOrigin()
	if self:IsPortalOriginEnabled() == false then
		local surfC = self:GetEntity():GetComponent(ents.COMPONENT_SURFACE)
		if surfC == nil then
			return Vector()
		end
		local plane = surfC:GetPlane()
		return plane:GetNormal() * plane:GetDistance()
	end
	return self.m_relativePortalOrigin
end

function Component:GetSurfacePose()
	self:UpdatePoses()
	return self.m_surfacePose
end
function Component:SetTargetPose(pose)
	self.m_targetPose = pose
end
function Component:GetTargetPose()
	self:UpdatePoses()
	return self.m_targetPose
end
function Component:GetTargetPlane()
	self:UpdatePoses()
	return self.m_targetPlane
end

local rot180Yaw = EulerAngles(0, 180, 0):ToQuaternion()
function Component:ProjectPoseToTarget(pose, planeSide, ignoreMirror)
	planeSide = planeSide or geometry.PLANE_SIDE_FRONT
	local newPose = self:GetSurfacePose():GetInverse() * pose
	local rot = newPose:GetRotation()
	if planeSide == geometry.PLANE_SIDE_FRONT then
		rot = rot180Yaw * rot
		if self:IsMirrored() then
			rot:MirrorAxis(math.AXIS_X)
		end
	end
	newPose:SetRotation(rot)
	local pos = newPose:GetOrigin()
	if planeSide == geometry.PLANE_SIDE_FRONT then
		if self:IsMirrored() == false then
			pos:Rotate(rot180Yaw)
		elseif ignoreMirror ~= true then
			pos.z = -pos.z
		end
	end
	newPose:SetOrigin(pos)
	return self:GetTargetPose() * newPose
end
ents.register_component("portal", Component, "gameplay", ents.EntityComponent.FREGISTER_BIT_NETWORKED)
Component.EVENT_ON_PORTAL_ORIGIN_CHANGED =
	ents.register_component_event(ents.COMPONENT_PORTAL, "on_portal_origin_changed")
