-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("ai.TaskMoveToPosition", ai.BaseBehaviorTask)
ai.TaskMoveToPosition.PARAM_MOVE_TARGET = 0
ai.TaskMoveToPosition.PARAM_MOVE_DISTANCE = 1
ai.TaskMoveToPosition.PARAM_MOVE_ACTIVITY = 2
ai.TaskMoveToPosition.PARAM_MAX_TIME = 3
ai.TaskMoveToPosition.PARAM_MOVE_ON_PATH = 4
function ai.TaskMoveToPosition:__tostring()
	return "Move to position..."
end
function ai.TaskMoveToPosition:__init(taskType, selectorType)
	ai.BaseBehaviorTask.__init(self, taskType, selectorType)
end

function ai.TaskMoveToPosition:CheckTime()
	if self.m_tEnd ~= nil and time.cur_time() > self.m_tEnd then
		return false
	end
	return true
end

function ai.TaskMoveToPosition:SetMovePosition(pos)
	self:SetParameterVector(self.PARAM_MOVE_TARGET, pos)
end

function ai.TaskMoveToPosition:SetMoveDistance(d)
	self:SetParameterFloat(self.PARAM_MOVE_DISTANCE, d)
end

function ai.TaskMoveToPosition:SetMoveActivity(act)
	self:SetParameterInt(self.PARAM_MOVE_ACTIVITY, act)
end

function ai.TaskMoveToPosition:GetMovePosition(schedule, npc)
	local pos = self:GetParameterVector(schedule, self.PARAM_MOVE_TARGET)
	local physComponent = npc:GetEntity():GetComponent(ents.COMPONENT_PHYSICS)
	return pos, (physComponent ~= nil) and physComponent:GetAABBDistance(pos) or 0
end

function ai.TaskMoveToPosition:Move(schedule, npc)
	local pos, dist = self:GetMovePosition(schedule, npc)
	if dist <= self.m_moveDistance then
		return ai.BehaviorTask.RESULT_SUCCEEDED
	end
	if self:CheckTime() == false then
		return ai.BehaviorTask.RESULT_FAILED
	end
	npc:MoveTo(pos, self.m_aiMoveInfo)
	return ai.BehaviorTask.RESULT_PENDING
end

function ai.TaskMoveToPosition:Think(schedule, npc)
	if self:HasParameter(schedule, self.PARAM_MOVE_TARGET) == false then
		return ai.BehaviorTask.RESULT_FAILED
	end
	return self:Move(schedule, npc)
end

function ai.TaskMoveToPosition:GetMoveDistance()
	return self.m_moveDistance or 0.0
end

function ai.TaskMoveToPosition:Start(schedule, npc)
	if self:HasParameter(schedule, self.PARAM_MOVE_TARGET) == false then
		return ai.BehaviorTask.RESULT_FAILED
	end
	self.m_moveDistance = self:GetParameterFloat(schedule, self.PARAM_MOVE_DISTANCE)
	self.m_tEnd = self:HasParameter(schedule, self.PARAM_MAX_TIME)
			and (time.cur_time() + self:GetParameterFloat(schedule, self.PARAM_MAX_TIME))
		or nil
	self.m_aiMoveInfo = ai.MoveInfo()
	self.m_aiMoveInfo.activity = self:GetParameterInt(schedule, self.PARAM_MOVE_ACTIVITY, Animation.ACT_WALK)
	self.m_aiMoveInfo.moveOnPath = self:GetParameterBool(schedule, self.PARAM_MOVE_ON_PATH)
	self.m_aiMoveInfo.destinationTolerance = math.max(self:GetMoveDistance() - 0.5, 0.1) -- Lower tolerance than actual distance to account for precision errors
	return ai.BehaviorTask.RESULT_PENDING
end
ai.TASK_MOVE_TO_POSITION = ai.TASK_MOVE_TO_POSITION or ai.register_task(ai.TaskMoveToPosition)
