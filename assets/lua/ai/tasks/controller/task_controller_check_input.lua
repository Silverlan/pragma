util.register_class("ai.TaskControllerCheckInput", ai.BaseBehaviorTask)
ai.TaskControllerCheckInput.PARAM_INPUT_ACTION = 0
function ai.TaskControllerCheckInput:__tostring()
	return "Check input action..."
end
function ai.TaskControllerCheckInput:__init(taskType, selectorType)
	ai.BaseBehaviorTask.__init(self, taskType, selectorType)
end

function ai.TaskControllerCheckInput:Start(schedule, npc)
	if self:HasParameter(schedule, 0) == false or npc:GetController() == nil then
		return ai.BehaviorTask.RESULT_FAILED
	end
	return (bit.band(npc:GetControllerActionInput(), self:GetParameterInt(schedule, 0)) ~= 0)
			and ai.BehaviorTask.RESULT_SUCCEEDED
		or ai.BehaviorTask.RESULT_FAILED
end
ai.TASK_CONTROLLER_CHECK_INPUT = ai.TASK_CONTROLLER_CHECK_INPUT or ai.register_task(ai.TaskControllerCheckInput)
