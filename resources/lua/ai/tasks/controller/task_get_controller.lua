util.register_class("ai.TaskGetController",ai.BaseBehaviorTask)
ai.TaskGetController.RETURN_CONTROLLER = 0
function ai.TaskGetController:__tostring() return "Get Controller" end
function ai.TaskGetController:__init(taskType,selectorType)
	ai.BaseBehaviorTask.__init(self,taskType,selectorType)
end

function ai.TaskGetController:Start(schedule,npc)
	local controller = npc:GetController()
	if(controller == nil) then return ai.BehaviorTask.RESULT_FAILED end
	self:SetParameterEntity(self.RETURN_CONTROLLER,controller)
	return ai.BehaviorTask.RESULT_SUCCEEDED
end
ai.TASK_GET_CONTROLLER = ai.TASK_GET_CONTROLLER or ai.register_task(ai.TaskGetController)
