util.register_class("ai.TaskLookInControllerDirection", ai.BaseBehaviorTask)
include("task_get_controller_look_target.lua")
ai.TaskLookInControllerDirection.PARAM_LOOK_DURATION = 0
function ai.TaskLookInControllerDirection:__tostring()
	return "Look in controller direction"
end
function ai.TaskLookInControllerDirection:__init(taskType, selectorType)
	ai.BaseBehaviorTask.__init(self, taskType, selectorType)

	local taskGetTarget = self:CreateTask(ai.TASK_GET_CONTROLLER_LOOK_TARGET)
	local taskLook = self:CreateTask(ai.TASK_LOOK_AT_TARGET)
	taskGetTarget:LinkParameter(ai.TaskGetControllerLookTarget.RETURN_LOOK_TARGET, taskLook, 0)
	self:LinkParameter(self.PARAM_LOOK_DURATION, taskLook, 1)
end
ai.TASK_LOOK_IN_CONTROLLER_DIRECTION = ai.TASK_LOOK_IN_CONTROLLER_DIRECTION
	or ai.register_task(ai.TaskLookInControllerDirection)
