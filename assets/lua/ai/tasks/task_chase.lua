-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("ai.TaskChase", ai.BaseBehaviorTask)
ai.TaskChase.PARAM_MOVE_ACTIVITY = 0
function ai.TaskChase:__tostring()
	return "Chase"
end
function ai.TaskChase:__init(taskType, selectorType)
	ai.BaseBehaviorTask.__init(self, taskType, selectorType)
end

function ai.TaskChase:Start(schedule, npc)
	local t = npc:GetPrimaryTarget()
	if t == nil then
		return ai.BehaviorTask.RESULT_FAILED
	end
	local moveInfo = ai.MoveInfo()
	local posDst = t:GetLastKnownPosition()
	local bInView = t:IsInView()
	if bInView == false then
		-- Target can't be seen; Attempt to find target by moving
		-- in last known direction
		-- (This doesn't work very well, NPCs tend to move into walls)
		--[[local dir = t:GetLastKnownVelocity()
		local speed = dir:Length()
		if(speed > 0.0) then
			dir = dir /speed
			speed = math.clamp(speed,50.0,200.0)
			local pos = npc:GetPos()
			pos = pos -posDst
			local posProj = pos:Project(dir)
			posProj = posProj +posDst -- NPC position projected to line of movement of target
			posProj = posProj +dir *speed -- Move to where the target is probably going to be

			posDst = posProj
		end]]
	end
	--[[if(npc.m_chaseTargetReached ~= nil and npc:TurnStep(posDst) == true) then
		npc.m_chaseTargetReached = nil
		return ai.BehaviorTask.RESULT_SUCCEEDED
	end]]
	local ent = npc:GetEntity()
	local trComponent = ent:GetTransformComponent()
	if trComponent ~= nil then
		local dist = trComponent:GetDistance(posDst)
		moveInfo.moveOnPath = (bInView == false or dist > 400.0) and true or false
		moveInfo.activity = self:GetParameterInt(schedule, self.PARAM_MOVE_ACTIVITY, Animation.ACT_RUN)
		local moveState = npc:MoveTo(posDst, moveInfo)
		if moveState == ai.MOVE_STATE_TARGET_UNREACHABLE then
			if moveInfo.moveOnPath == true then
				moveInfo.moveOnPath = false
				moveState = npc:MoveTo(posDst, moveInfo) -- Try to move without path
				if moveState == ai.MOVE_STATE_TARGET_UNREACHABLE then
					npc:StopMoving()
					-- TODO: Try to hide?
					return ai.BehaviorTask.RESULT_FAILED -- We cannot reach our target (Path blocked?)
				end
			end
		end
	end
	--[[if(moveState == ai.MOVE_STATE_TARGET_REACHED) then
		npc.m_chaseTargetReached = time.cur_time()
		if(npc:IsMoving() == true) then npc:StopMoving() end
		--npc:TurnStep(posDst)
	end]]
	return ai.BehaviorTask.RESULT_SUCCEEDED
end
ai.TASK_CHASE = ai.TASK_CHASE or ai.register_task(ai.TaskChase)
