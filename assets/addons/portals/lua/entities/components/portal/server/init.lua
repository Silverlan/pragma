-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include("../shared.lua")

local Component = ents.PortalComponent

function Component:OnTick(dt)
	local touchC = self:GetEntity():GetComponent(ents.COMPONENT_TOUCH)
	if touchC == nil then
		return
	end
	local surfC = self:GetEntity():GetComponent(ents.COMPONENT_SURFACE)
	if surfC == nil then
		return
	end
	local plane = surfC:GetPlaneWs()
	for _, ent in ipairs(touchC:GetTouchingEntities()) do
		local n = plane:GetNormal()
		local d = plane:GetDistance()
		local pos = ent:GetPos()
		local side = geometry.get_side_of_point_to_plane(n, d, pos)
		if side == geometry.PLANE_SIDE_FRONT then
			local posNext = pos + ent:GetVelocity() * dt
			side = geometry.get_side_of_point_to_plane(n, d, posNext)
			if side == geometry.PLANE_SIDE_BACK then
				-- Teleport the player
				local nDst = self:GetTargetPlane():GetNormal()
				local dDst = self:GetTargetPlane():GetDistance()

				local srcPos = self:GetSurfacePose():GetOrigin()
				local targetPos = self:GetTargetPose():GetOrigin()
				local srcRot = self:GetSurfacePose():GetRotation()
				local targetRot = self:GetTargetPose():GetRotation()
				local srcPose = math.Transform(srcPos, srcRot)
				local targetPose = math.Transform(targetPos, targetRot)

				local newPose = self:ProjectPoseToTarget(ent:GetPose())

				local tC = ent:GetComponent(ents.COMPONENT_TRANSFORM)
				if tC ~= nil then
					tC:Teleport(newPose)
				end
			end
		end
	end
end

function Component:UpdateMirrorState()
	local physC = self:GetEntity():GetComponent(ents.COMPONENT_PHYSICS)
	if physC ~= nil then
		physC:SetCollisionFilterGroup(mirrored and phys.COLLISIONMASK_STATIC or phys.COLLISIONMASK_TRIGGER)
		physC:SetCollisionCallbacksEnabled(not mirrored)
	end

	local mirrored = self:IsMirrored()
	if mirrored then
		self:GetEntity():RemoveComponent(ents.COMPONENT_TOUCH)
		self:SetTickPolicy(ents.TICK_POLICY_NEVER)
	else
		self:GetEntity():AddComponent(ents.COMPONENT_TOUCH)
	end
end

function Component:OnStartTouch(ent)
	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
end

function Component:OnEndTouch(ent)
	local touchC = self:GetEntity():GetComponent(ents.COMPONENT_TOUCH)
	if touchC:GetTouchingEntityCount() == 0 then
		self:SetTickPolicy(ents.TICK_POLICY_NEVER)
	end
end

function Component:CanTrigger(ent, physObj)
	return util.EVENT_REPLY_HANDLED, ent:IsPlayer()
end

function Component:OnPhysicsInitialized(physObj)
	local physComponent = self:GetEntity():GetComponent(ents.COMPONENT_PHYSICS)
	if physComponent == nil then
		return
	end
	physComponent:SetCollisionFilterMask(phys.COLLISIONMASK_PLAYER)
	self:UpdateMirrorState()
end

function Component:OnEntitySpawn()
	local ent = self:GetEntity()
	local touchComponent = ent:GetComponent(ents.COMPONENT_TOUCH)
	if touchComponent ~= nil then
		touchComponent:SetTriggerFlags(ents.TouchComponent.TRIGGER_FLAG_BIT_PLAYERS)
	end
	local physComponent = ent:GetPhysicsComponent()
	if physComponent ~= nil then
		physComponent:InitializePhysics(phys.TYPE_STATIC)
	end
end
