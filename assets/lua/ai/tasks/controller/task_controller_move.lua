util.register_class("ai.TaskControllerMove", ai.BaseBehaviorTask)
function ai.TaskControllerMove:__tostring()
	return "Move..."
end
function ai.TaskControllerMove:__init(taskType, selectorType)
	ai.BaseBehaviorTask.__init(self, taskType, selectorType)
end

function ai.TaskControllerMove:GetMovementDirection(npc)
	local trComponent = npc:GetEntity():GetTransformComponent()
	if trComponent == nil then
		return Vector(), 0
	end
	local controller = npc:GetController()
	local charComponentCtrl = (controller ~= nil) and controller:GetCharacterComponent() or nil
	if charComponentCtrl == nil then
		return trComponent:GetForward(), 1.0
	end
	local dir = Vector()
	local actionInput = npc:GetControllerActionInput()
	if bit.band(actionInput, input.ACTION_MOVEFORWARD) ~= 0 then
		dir = dir + charComponentCtrl:GetViewForward()
	end
	if bit.band(actionInput, input.ACTION_MOVEBACKWARD) ~= 0 then
		dir = dir - charComponentCtrl:GetViewForward()
	end
	if bit.band(actionInput, input.ACTION_MOVERIGHT) ~= 0 then
		dir = dir + charComponentCtrl:GetViewRight()
	end
	if bit.band(actionInput, input.ACTION_MOVELEFT) ~= 0 then
		dir = dir - charComponentCtrl:GetViewRight()
	end
	local l = dir:Length()
	if l > 0.0 then
		dir = dir / l
	end
	return dir, l
end

function ai.TaskControllerMove:Start(schedule, npc)
	local dir, l = self:GetMovementDirection(npc)
	local trComponent = npc:GetEntity():GetTransformComponent()
	if l == 0.0 or trComponent == nil then
		npc:StopMoving()
		return ai.BehaviorTask.RESULT_FAILED
	end
	local moveInfo = ai.MoveInfo()
	moveInfo.activity = Animation.ACT_WALK
	if bit.band(npc:GetControllerActionInput(), input.ACTION_SPRINT) ~= 0 then
		moveInfo.activity = Animation.ACT_RUN
	end
	moveInfo.moveOnPath = false
	npc:MoveTo(trComponent:GetPos() + dir * 200.0, moveInfo)
	return ai.BehaviorTask.RESULT_SUCCEEDED
end
ai.TASK_CONTROLLER_MOVE = ai.TASK_CONTROLLER_MOVE or ai.register_task(ai.TaskControllerMove)
