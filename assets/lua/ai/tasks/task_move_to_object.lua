include("task_move_to_position.lua")
util.register_class("ai.TaskMoveToObject",ai.TaskMoveToPosition)
function ai.TaskMoveToObject:__tostring() return "Move to object..." end
function ai.TaskMoveToObject:__init(taskType,selectorType)
	ai.TaskMoveToPosition.__init(self,taskType,selectorType)
end

function ai.TaskMoveToObject:GetMovePosition(schedule,npc)
	local ent = self:GetMoveEntity(schedule)
	local trComponent = ent:GetTransformComponent()
	local charComponentNpc = npc:GetCharacterComponent()
	local origin = (trComponent ~= nil) and trComponent:GetCenter() or Vector()
	local dist = (charComponentNpc ~= nil) and charComponentNpc:GetAABBDistance(ent) or 0
	return origin,dist
end

function ai.TaskMoveToObject:GetMoveEntity(schedule)
	return self:GetParameterEntity(schedule,self.PARAM_MOVE_TARGET)
end

function ai.TaskMoveToObject:Start(schedule,npc)
	if(self:GetParameterType(schedule,self.PARAM_MOVE_TARGET) ~= ai.SCHEDULE_PARAMETER_TYPE_ENTITY) then return ai.BehaviorTask.RESULT_FAILED end
	return ai.TaskMoveToPosition.Start(self,schedule,npc)
end
ai.TASK_MOVE_TO_OBJECT = ai.TASK_MOVE_TO_OBJECT or ai.register_task(ai.TaskMoveToObject)
