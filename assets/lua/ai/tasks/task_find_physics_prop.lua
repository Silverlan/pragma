-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("ai.TaskFindPhysicsProp", ai.BaseBehaviorTask)
ai.TaskFindPhysicsProp.PARAM_MAX_DISTANCE = 0
ai.TaskFindPhysicsProp.PARAM_MAX_MASS = 1

ai.TaskFindPhysicsProp.RETURN_PHYSICS_PROP = 2
function ai.TaskFindPhysicsProp:__tostring()
	return "Find Physics Prop..."
end
function ai.TaskFindPhysicsProp:__init(taskType, selectorType)
	ai.BaseBehaviorTask.__init(self, taskType, selectorType)
end

function ai.TaskFindPhysicsProp:FindPhysicsPropCandidate(schedule, npc)
	local maxDist = self:GetParameterFloat(schedule, self.PARAM_MAX_DISTANCE, 500.0)
	local maxMass = self:GetParameterFloat(schedule, self.PARAM_MAX_MASS, 50.0)
	local ent = npc:GetEntity()
	local trComponent = ent:GetTransformComponent()
	local charComponent = ent:GetCharacterComponent()
	if trComponent == nil or charComponent == nil then
		return
	end
	local origin = ent:GetCenter()
	local tr = charComponent:GetAimRayData()
	tr:SetSource(origin)
	local entClosest
	local distClosest = math.huge
	for ent in
		ents.iterator(
			bit.bor(ents.ITERATOR_FILTER_DEFAULT, ents.ITERATOR_FILTER_BIT_PHYSICAL),
			{ ents.IteratorFilterSphere(origin, maxDist) }
		)
	do
		local physComponent = ent:GetPhysicsComponent()
		if physComponent:IsPhysicsProp() == true then
			local physObj = physComponent:GetPhysicsObject()
			if physObj ~= nil and math.is_in_range(physObj:GetMass(), 0.0, maxMass) then
				local trComponent = ent:GetTransformComponent()
				local posEnt = ent:GetCenter()
				local distSqr = posEnt:DistanceSqr(origin)
				if distSqr < distClosest then
					tr:SetTarget(posEnt)
					tr:SetFilter(ent)
					tr:SetFlags(phys.FTRACE_FILTER_INVERT)
					local r = phys.raycast(tr) -- Make reasonably sure the path to the object is clear
					if r == false or r.entity == ent then
						distClosest = distSqr
						entClosest = ent
					end
				end
			end
		end
	end
	return entClosest
end

function ai.TaskFindPhysicsProp:Start(schedule, npc)
	local entPhys = self:FindPhysicsPropCandidate(schedule, npc)
	if entPhys == nil then
		return ai.BehaviorTask.RESULT_FAILED
	end
	self:SetParameterEntity(self.RETURN_PHYSICS_PROP, entPhys)
	return ai.BehaviorTask.RESULT_SUCCEEDED
end
ai.TASK_FIND_PHYSICS_PROP = ai.TASK_FIND_PHYSICS_PROP or ai.register_task(ai.TaskFindPhysicsProp)
